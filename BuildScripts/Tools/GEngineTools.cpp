#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <objbase.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "shlwapi.lib")

namespace
{
    using Path = std::filesystem::path;

    struct ProcessResult
    {
        int ExitCode = -1;
        std::string Output;
    };

    struct Sha256
    {
        std::array<unsigned char, 32> Value{};
    };

    struct ProjectSpec
    {
        std::wstring Name;
        Path Project;
        Path Root;
    };

    struct ExportOptions
    {
        std::wstring ExecutableName;
        Path Icon;
    };

    std::wstring Trim(std::wstring value)
    {
        const auto first = value.find_first_not_of(L" \t\r\n");
        if (first == std::wstring::npos) return {};
        const auto last = value.find_last_not_of(L" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    std::string WideToUtf8(const std::wstring& value)
    {
        if (value.empty()) return {};
        const auto size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()),
            nullptr, 0, nullptr, nullptr);
        std::string result(static_cast<std::size_t>(size), '\0');
        WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), result.data(),
            size, nullptr, nullptr);
        return result;
    }

    std::wstring Utf8ToWide(std::string_view value)
    {
        if (value.empty()) return {};
        const auto size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
            static_cast<int>(value.size()), nullptr, 0);
        std::wstring result(static_cast<std::size_t>(size), L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()),
            result.data(), size);
        return result;
    }

    std::wstring Quote(const std::wstring& value)
    {
        std::wstring result = L"\"";
        for (const auto character : value)
        {
            if (character == L'"') result += L"\\\"";
            else result += character;
        }
        return result + L"\"";
    }

    std::wstring JoinArguments(const std::vector<std::wstring>& arguments)
    {
        std::wstring result;
        for (const auto& argument : arguments)
        {
            if (!result.empty()) result += L' ';
            result += Quote(argument);
        }
        return result;
    }

    std::string Hex(const std::array<unsigned char, 32>& value)
    {
        static constexpr char digits[] = "0123456789abcdef";
        std::string result;
        result.reserve(value.size() * 2);
        for (const auto byte : value) result.append({digits[byte >> 4], digits[byte & 15]});
        return result;
    }

    std::string Hex(const Sha256& value)
    {
        return Hex(value.Value);
    }

    std::wstring Lowercase(std::wstring value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](wchar_t character)
        {
            return static_cast<wchar_t>(towlower(character));
        });
        return value;
    }

    std::string Trim(std::string value)
    {
        const auto first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return {};
        const auto last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    std::wstring RandomId()
    {
        std::array<unsigned char, 16> bytes{};
        if (BCryptGenRandom(nullptr, bytes.data(), static_cast<ULONG>(bytes.size()),
            BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0)
        {
            throw std::runtime_error("Cannot generate a staging identity.");
        }
        static constexpr wchar_t digits[] = L"0123456789abcdef";
        std::wstring result;
        result.reserve(bytes.size() * 2);
        for (const auto byte : bytes)
        {
            result += digits[byte >> 4];
            result += digits[byte & 15];
        }
        return result;
    }

    Path RepoRoot()
    {
        std::wstring module(32768, L'\0');
        const auto length = GetModuleFileNameW(nullptr, module.data(), static_cast<DWORD>(module.size()));
        if (!length || length >= module.size()) throw std::runtime_error("Cannot locate GEngineTools.");
        module.resize(length);
        auto current = Path(module).parent_path();
        for (; current.has_parent_path(); current = current.parent_path())
        {
            if (std::filesystem::exists(current / L"BuildScripts" / L"Project" / L"CMakeLists.txt")) return current;
        }
        return std::filesystem::current_path();
    }

    Path TemporaryPath()
    {
        std::wstring value(32768, L'\0');
        const auto length = GetTempPathW(static_cast<DWORD>(value.size()), value.data());
        if (!length || length >= value.size()) throw std::runtime_error("Cannot locate the temporary directory.");
        value.resize(length);
        return Path(value);
    }

    std::wstring SearchTool(const std::wstring& name)
    {
        std::wstring buffer(32768, L'\0');
        const auto length = SearchPathW(nullptr, name.c_str(), nullptr, static_cast<DWORD>(buffer.size()),
            buffer.data(), nullptr);
        if (!length || length >= buffer.size()) throw std::runtime_error("Cannot find " + WideToUtf8(name) + ".");
        buffer.resize(length);
        return buffer;
    }

    Path FindCMake() { return Path(SearchTool(L"cmake.exe")); }

    ProcessResult RunProcess(const std::wstring& executable, const std::vector<std::wstring>& arguments,
        const std::optional<Path>& workingDirectory = {});

    Path FindVisualStudio()
    {
        wchar_t programFiles[32768]{};
        if (!GetEnvironmentVariableW(L"ProgramFiles(x86)", programFiles, 32768))
        {
            throw std::runtime_error("Cannot locate Visual Studio Installer.");
        }
        const Path vswhere = Path(programFiles) / L"Microsoft Visual Studio" / L"Installer" / L"vswhere.exe";
        if (!std::filesystem::exists(vswhere)) throw std::runtime_error("Visual Studio vswhere is missing.");
        const auto result = RunProcess(vswhere.wstring(), {L"-latest", L"-products", L"*",
            L"-requires", L"Microsoft.VisualStudio.Component.VC.Tools.x86.x64", L"-property", L"installationPath"});
        if (result.ExitCode != 0) throw std::runtime_error("Visual Studio discovery failed.");
        std::istringstream output(result.Output);
        std::string line;
        while (std::getline(output, line))
        {
            const auto value = Trim(Utf8ToWide(line));
            if (!value.empty()) return Path(value);
        }
        throw std::runtime_error("Visual Studio C++ tools are required.");
    }

    Path VcVarsPath(const Path& visualStudio)
    {
        const auto result = visualStudio / L"VC" / L"Auxiliary" / L"Build" / L"vcvars64.bat";
        if (!std::filesystem::exists(result)) throw std::runtime_error("Visual Studio vcvars64.bat is missing.");
        return result;
    }

    ProcessResult RunRaw(const std::wstring& executable, const std::wstring& commandLine,
        const std::optional<Path>& workingDirectory = {})
    {
        SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
        HANDLE readPipe = nullptr;
        HANDLE writePipe = nullptr;
        if (!CreatePipe(&readPipe, &writePipe, &security, 65536))
        {
            throw std::runtime_error("Cannot create a process output pipe.");
        }
        SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);
        STARTUPINFOW startup{};
        startup.cb = sizeof(startup);
        startup.dwFlags = STARTF_USESTDHANDLES;
        startup.hStdOutput = writePipe;
        startup.hStdError = writePipe;
        startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        PROCESS_INFORMATION process{};
        std::wstring mutableCommandLine = commandLine;
        const auto working = workingDirectory ? workingDirectory->wstring() : std::wstring{};
        const auto created = CreateProcessW(executable.c_str(), mutableCommandLine.data(), nullptr, nullptr, TRUE,
            CREATE_NO_WINDOW, nullptr, working.empty() ? nullptr : working.c_str(), &startup, &process);
        CloseHandle(writePipe);
        if (!created)
        {
            CloseHandle(readPipe);
            throw std::runtime_error("Cannot start " + WideToUtf8(executable) + ".");
        }
        std::string output;
        std::array<char, 65536> buffer{};
        DWORD bytesRead = 0;
        while (ReadFile(readPipe, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, nullptr) && bytesRead)
        {
            output.append(buffer.data(), bytesRead);
        }
        CloseHandle(readPipe);
        WaitForSingleObject(process.hProcess, INFINITE);
        DWORD exitCode = 0;
        GetExitCodeProcess(process.hProcess, &exitCode);
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        return {static_cast<int>(exitCode), std::move(output)};
    }

    ProcessResult RunProcess(const std::wstring& executable, const std::vector<std::wstring>& arguments,
        const std::optional<Path>& workingDirectory)
    {
        return RunRaw(executable, L"\"" + executable + L"\" " + JoinArguments(arguments), workingDirectory);
    }

    void WriteLog(const Path& path, const std::string& output)
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file) throw std::runtime_error("Cannot write log: " + WideToUtf8(path.wstring()));
        file.write(output.data(), static_cast<std::streamsize>(output.size()));
    }

    ProcessResult RunLogged(const std::wstring& executable, const std::vector<std::wstring>& arguments,
        const Path& workingDirectory, const Path& log, bool offscreen = false)
    {
        ProcessResult result;
        if (offscreen)
        {
            wchar_t commandSpec[32768]{};
            if (!GetEnvironmentVariableW(L"ComSpec", commandSpec, 32768))
            {
                throw std::runtime_error("Cannot locate cmd.exe.");
            }
            const auto command = L"/d /s /c \"set QT_QPA_PLATFORM=offscreen&& " + Quote(executable) + L" " +
                JoinArguments(arguments) + L"\"";
            result = RunRaw(commandSpec, command, workingDirectory);
        }
        else result = RunProcess(executable, arguments, workingDirectory);
        WriteLog(log, result.Output);
        std::cout << WideToUtf8(log.filename().wstring()) << " : exit " << result.ExitCode
            << " (" << WideToUtf8(log.wstring()) << ")\n";
        if (result.ExitCode != 0)
        {
            std::cerr << result.Output;
            throw std::runtime_error("Command failed. Inspect " + WideToUtf8(log.wstring()));
        }
        return result;
    }

    ProcessResult RunVcLogged(const Path& visualStudio, const std::wstring& executable,
        const std::vector<std::wstring>& arguments, const Path& workingDirectory, const Path& log)
    {
        wchar_t commandSpec[32768]{};
        if (!GetEnvironmentVariableW(L"ComSpec", commandSpec, static_cast<DWORD>(std::size(commandSpec))))
        {
            throw std::runtime_error("Cannot locate cmd.exe.");
        }
        const auto vcvars = VcVarsPath(visualStudio).wstring();
        const auto command = L"/d /s /c \"call " + Quote(vcvars) + L" >nul && " + Quote(executable) + L" " +
            JoinArguments(arguments) + L"\"";
        auto result = RunRaw(commandSpec, command, workingDirectory);
        WriteLog(log, result.Output);
        std::cout << WideToUtf8(log.filename().wstring()) << " : exit " << result.ExitCode
            << " (" << WideToUtf8(log.wstring()) << ")\n";
        if (result.ExitCode != 0)
        {
            std::cerr << result.Output;
            throw std::runtime_error("Command failed. Inspect " + WideToUtf8(log.wstring()));
        }
        return result;
    }

    Sha256 HashBytes(const std::string& value)
    {
        BCRYPT_ALG_HANDLE algorithm = nullptr;
        BCRYPT_HASH_HANDLE hash = nullptr;
        if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
        {
            throw std::runtime_error("SHA-256 is unavailable.");
        }
        if (BCryptCreateHash(algorithm, &hash, nullptr, 0, nullptr, 0, 0) < 0)
        {
            BCryptCloseAlgorithmProvider(algorithm, 0);
            throw std::runtime_error("SHA-256 initialization failed.");
        }
        Sha256 result{};
        const auto update = BCryptHashData(hash, reinterpret_cast<unsigned char*>(const_cast<char*>(value.data())),
            static_cast<ULONG>(value.size()), 0);
        const auto finish = update >= 0 ? BCryptFinishHash(hash, result.Value.data(),
            static_cast<ULONG>(result.Value.size()), 0) : -1;
        BCryptDestroyHash(hash);
        BCryptCloseAlgorithmProvider(algorithm, 0);
        if (update < 0 || finish < 0) throw std::runtime_error("SHA-256 hashing failed.");
        return result;
    }

    Sha256 HashFile(const Path& path)
    {
        BCRYPT_ALG_HANDLE algorithm = nullptr;
        BCRYPT_HASH_HANDLE hash = nullptr;
        if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
        {
            throw std::runtime_error("SHA-256 is unavailable.");
        }
        if (BCryptCreateHash(algorithm, &hash, nullptr, 0, nullptr, 0, 0) < 0)
        {
            BCryptCloseAlgorithmProvider(algorithm, 0);
            throw std::runtime_error("SHA-256 initialization failed.");
        }
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot hash " + WideToUtf8(path.wstring()));
        std::array<char, 65536> buffer{};
        bool failed = false;
        while (file)
        {
            file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
            const auto count = static_cast<ULONG>(file.gcount());
            if (count && BCryptHashData(hash, reinterpret_cast<unsigned char*>(buffer.data()), count, 0) < 0)
            {
                failed = true;
                break;
            }
        }
        Sha256 result{};
        if (!failed && BCryptFinishHash(hash, result.Value.data(), static_cast<ULONG>(result.Value.size()), 0) < 0)
        {
            failed = true;
        }
        BCryptDestroyHash(hash);
        BCryptCloseAlgorithmProvider(algorithm, 0);
        if (failed || !file.eof()) throw std::runtime_error("Cannot hash " + WideToUtf8(path.wstring()));
        return result;
    }

    bool IsReparsePoint(const Path& path)
    {
        const auto attributes = GetFileAttributesW(path.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_REPARSE_POINT);
    }

    void AssertNoLinks(const Path& path)
    {
        auto current = std::filesystem::absolute(path);
        while (!current.empty())
        {
            if (std::filesystem::exists(current) && IsReparsePoint(current))
            {
                throw std::runtime_error("Linked path rejected: " + WideToUtf8(current.wstring()));
            }
            if (!current.has_parent_path() || current.parent_path() == current) break;
            current = current.parent_path();
        }
    }

    void RemoveTree(const Path& path, const Path& allowedRoot, const std::wstring& allowedPrefix)
    {
        if (!std::filesystem::exists(path)) return;
        const auto full = std::filesystem::absolute(path);
        const auto root = std::filesystem::absolute(allowedRoot);
        auto relative = std::filesystem::relative(full, root);
        if (relative.empty() || relative.native().find(L"..") == 0 || relative.is_absolute() ||
            _wcsnicmp(relative.c_str(), allowedPrefix.c_str(), allowedPrefix.size()) != 0)
        {
            throw std::runtime_error("Refusing unsafe cleanup: " + WideToUtf8(full.wstring()));
        }
        std::error_code error;
        std::filesystem::remove_all(full, error);
        if (error) throw std::runtime_error("Cleanup failed: " + error.message());
    }

    bool Within(const Path& child, const Path& parent)
    {
        auto normalizedChild = std::filesystem::weakly_canonical(child);
        auto normalizedParent = std::filesystem::weakly_canonical(parent);
        if (normalizedChild == normalizedParent) return true;
        auto relative = std::filesystem::relative(normalizedChild, normalizedParent);
        return !relative.empty() && !relative.is_absolute() && relative.native().find(L"..") != 0;
    }

    class ProcessJob
    {
    public:
        ProcessJob()
        {
            job_ = CreateJobObjectW(nullptr, nullptr);
            if (!job_) throw std::runtime_error("Cannot create the compiler cleanup job.");
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION information{};
            information.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            if (!SetInformationJobObject(job_, JobObjectExtendedLimitInformation, &information, sizeof(information)) ||
                !AssignProcessToJobObject(job_, GetCurrentProcess()))
            {
                CloseHandle(job_);
                job_ = nullptr;
                throw std::runtime_error("Cannot establish compiler process cleanup.");
            }
        }
        ~ProcessJob()
        {
            if (job_)
            {
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION information{};
                SetInformationJobObject(job_, JobObjectExtendedLimitInformation, &information, sizeof(information));
                CloseHandle(job_);
            }
        }
    private:
        HANDLE job_ = nullptr;
    };

    class ProjectMutex
    {
    public:
        explicit ProjectMutex(const Path& project)
        {
            const auto identity = Hex(HashBytes(WideToUtf8(Lowercase(
                std::filesystem::absolute(project).wstring()))));
            const std::wstring name = L"Local\\GEngineProject-" + Utf8ToWide(identity);
            handle_ = CreateMutexW(nullptr, FALSE, name.c_str());
            if (!handle_) throw std::runtime_error("Cannot create the project packaging mutex.");
            const auto waited = WaitForSingleObject(handle_, 0);
            if (waited == WAIT_OBJECT_0 || waited == WAIT_ABANDONED) owned_ = true;
        }
        ~ProjectMutex()
        {
            if (handle_)
            {
                if (owned_) ReleaseMutex(handle_);
                CloseHandle(handle_);
            }
        }
        void RequireOwned()
        {
            if (!owned_) throw std::runtime_error("This project is already being compiled/packaged.");
        }
    private:
        HANDLE handle_ = nullptr;
        bool owned_ = false;
    };

    class OutputLock
    {
    public:
        OutputLock(const Path& parent, const Path& output)
        {
            const auto identity = Hex(HashBytes(WideToUtf8(Lowercase(
                std::filesystem::absolute(output).wstring()))));
            path_ = parent / (L".gengine-publish-" + Utf8ToWide(identity) + L".lock");
            handle_ = CreateFileW(path_.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE,
                nullptr);
            if (handle_ == INVALID_HANDLE_VALUE)
            {
                handle_ = nullptr;
                throw std::runtime_error("Cannot lock the publication output.");
            }
        }
        ~OutputLock()
        {
            if (handle_) CloseHandle(handle_);
        }
    private:
        Path path_;
        HANDLE handle_ = nullptr;
    };

    std::vector<std::pair<std::string, std::string>> ParseFlatJsonObject(const std::string& text)
    {
        std::vector<std::pair<std::string, std::string>> result;
        std::size_t position = text.find_first_not_of(" \t\r\n");
        if (position == std::string::npos || text[position] != '{') throw std::runtime_error("Expected JSON object.");
        ++position;
        while (true)
        {
            position = text.find_first_not_of(" \t\r\n", position);
            if (position == std::string::npos) throw std::runtime_error("Truncated JSON object.");
            if (text[position] == '}') return result;
            if (text[position] != ',') if (!result.empty()) throw std::runtime_error("Expected JSON comma.");
            if (text[position] == ',')
            {
                ++position;
                position = text.find_first_not_of(" \t\r\n", position);
                if (position == std::string::npos) throw std::runtime_error("Truncated JSON object.");
            }
            if (text[position] != '"') throw std::runtime_error("Expected JSON key.");
            const auto keyStart = ++position;
            while (position < text.size() && text[position] != '"')
            {
                if (text[position] == '\\') ++position;
                ++position;
            }
            if (position >= text.size()) throw std::runtime_error("Truncated JSON key.");
            const std::string key = text.substr(keyStart, position++ - keyStart);
            position = text.find_first_not_of(" \t\r\n", position);
            if (position == std::string::npos || text[position] != ':') throw std::runtime_error("Expected JSON colon.");
            position = text.find_first_not_of(" \t\r\n", ++position);
            if (position == std::string::npos) throw std::runtime_error("Truncated JSON value.");
            std::size_t end = position;
            if (text[position] == '"')
            {
                const auto valueStart = position;
                end = ++position;
                while (end < text.size() && text[end] != '"')
                {
                    if (text[end] == '\\') ++end;
                    ++end;
                }
                if (end >= text.size()) throw std::runtime_error("Truncated JSON string.");
                ++end;
                result.emplace_back(key, text.substr(valueStart, end - valueStart));
            }
            else
            {
                while (end < text.size() && text[end] != ',' && text[end] != '}') ++end;
                result.emplace_back(key, Trim(std::string(text.substr(position, end - position))));
            }
            position = end;
        }
    }

    std::string JsonEscape(const std::string& value)
    {
        std::string result;
        for (const auto character : value)
        {
            switch (character)
            {
            case '\\': result += "\\\\"; break;
            case '"': result += "\\\""; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (static_cast<unsigned char>(character) < 0x20)
                {
                    static constexpr char digits[] = "0123456789abcdef";
                    result += "\\u00";
                    result += digits[(static_cast<unsigned char>(character) >> 4) & 15];
                    result += digits[static_cast<unsigned char>(character) & 15];
                }
                else result += character;
            }
        }
        return result;
    }

    std::string ReadText(const Path& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot read " + WideToUtf8(path.wstring()));
        std::ostringstream stream;
        stream << file.rdbuf();
        return stream.str();
    }

    void WriteText(const Path& path, const std::string& text)
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        if (!file) throw std::runtime_error("Cannot write " + WideToUtf8(path.wstring()));
        file.write(text.data(), static_cast<std::streamsize>(text.size()));
    }

    bool ValidProjectName(const std::wstring& name)
    {
        if (name.empty() || name.size() > 101 || name.back() == L'.' || name.back() == L' ') return false;
        if (name.find_first_of(L"<>:\"/\\|?*") != std::wstring::npos) return false;
        for (const auto character : name)
        {
            if (character < 32) return false;
        }
        const auto first = name.front();
        if (!(iswalnum(first) || first == L'_' || first == L'-')) return false;
        return true;
    }

    ProjectSpec LoadProject(const Path& project)
    {
        AssertNoLinks(project);
        if (!std::filesystem::exists(project) || _wcsicmp(project.extension().c_str(), L".gproject") != 0)
        {
            throw std::runtime_error("Choose a valid .gproject file.");
        }
        if (std::filesystem::file_size(project) > 1024 * 1024)
        {
            throw std::runtime_error("Project file exceeds 1 MiB.");
        }
        const auto fields = ParseFlatJsonObject(ReadText(project));
        if (fields.size() != 2) throw std::runtime_error("Project files contain project metadata only.");
        bool hasVersion = false;
        std::optional<std::wstring> name;
        for (const auto& field : fields)
        {
            if (field.first == "schemaVersion") hasVersion = field.second == "1";
            else if (field.first == "name")
            {
                const auto value = Utf8ToWide(field.second);
                if (value.size() < 2 || value.front() != '"' || value.back() != '"')
                {
                    throw std::runtime_error("Project name must be a string.");
                }
                name = Trim(value.substr(1, value.size() - 2));
            }
            else throw std::runtime_error("Project files contain project metadata only.");
        }
        if (!hasVersion || !name || !ValidProjectName(*name))
        {
            throw std::runtime_error("Invalid project name or schema version.");
        }
        return {*name, std::filesystem::absolute(project), std::filesystem::absolute(project.parent_path())};
    }

    Path BuildProject(const Path& repoRoot, const ProjectSpec& project, const ExportOptions& options)
    {
        if (!std::filesystem::exists(project.Root / L"src"))
        {
            throw std::runtime_error("Project src/ directory is missing.");
        }
        const auto visualStudio = FindVisualStudio();
        const auto cmake = FindCMake().wstring();
        const auto ninja = (visualStudio / L"Common7" / L"IDE" / L"CommonExtensions" / L"Microsoft" / L"CMake" /
            L"Ninja" / L"ninja.exe").wstring();
        if (!std::filesystem::exists(Path(ninja))) throw std::runtime_error("Visual Studio Ninja is missing.");
        const auto identity = Hex(HashBytes(WideToUtf8(Lowercase(
            std::filesystem::absolute(project.Root).wstring()))));
        const auto cache = TemporaryPath() / L"GEngineBuilds" / Utf8ToWide(identity);
        std::filesystem::create_directories(cache);
        const auto source = (repoRoot / L"BuildScripts" / L"Project").generic_wstring();
        const auto engine = repoRoot.generic_wstring();
        const auto projectRoot = project.Root.generic_wstring();
        const auto nativeRoot = (cache / L"native").generic_wstring();
        const auto buildRoot = cache.generic_wstring();
        const auto makeProgram = Path(ninja).generic_wstring();
        std::vector<std::wstring> configureArguments{L"-S", source, L"-B", buildRoot, L"-G", L"Ninja",
            L"-DCMAKE_MAKE_PROGRAM=" + makeProgram, L"-DCMAKE_BUILD_TYPE=Release",
            L"-DGE_ENGINE_ROOT=" + engine, L"-DGE_PROJECT_ROOT=" + projectRoot,
            L"-DGE_PROJECT_NAME=" + options.ExecutableName, L"-DGE_DIST_ROOT=" + nativeRoot};
        Path cachedIcon;
        if (!options.Icon.empty())
        {
            cachedIcon = cache / L"launcher.ico";
            if (!std::filesystem::exists(cachedIcon) ||
                Hex(HashFile(cachedIcon)) != Hex(HashFile(options.Icon)))
            {
                std::filesystem::copy_file(options.Icon, cachedIcon,
                    std::filesystem::copy_options::overwrite_existing);
            }
        }
        configureArguments.push_back(L"-DGE_PROJECT_ICON=" + cachedIcon.generic_wstring());
        RunVcLogged(visualStudio, cmake, configureArguments, repoRoot, cache / L"configure.log");
        RunVcLogged(visualStudio, cmake, {L"--build", buildRoot, L"--target", L"GProjectPackage",
            L"--parallel", L"2"}, repoRoot, cache / L"build.log");
        const auto output = cache / L"native" / L"Release" / L"Game";
        if (!std::filesystem::exists(output / (options.ExecutableName + L".exe")) ||
            !std::filesystem::exists(output / L"Runtime" / (options.ExecutableName + L".exe")))
        {
            throw std::runtime_error("Compilation did not produce the launcher and game executable.");
        }
        return output;
    }

    void AppendLittle(std::ostream& stream, const void* bytes, std::size_t count)
    {
        stream.write(static_cast<const char*>(bytes), static_cast<std::streamsize>(count));
    }

    void AppendLittle32(std::ostream& stream, std::uint32_t value)
    {
        AppendLittle(stream, &value, sizeof(value));
    }

    void AppendLittle64(std::ostream& stream, std::uint64_t value)
    {
        AppendLittle(stream, &value, sizeof(value));
    }

    void WriteResourcePack(const ProjectSpec& project, const Path& pack)
    {
        struct Entry
        {
            std::string RelativePath;
            std::filesystem::path SourcePath;
            std::uint64_t Size = 0;
            std::optional<std::string> Content;
        };
        std::vector<Entry> entries;
        for (const auto& name : {L"Assets", L"Licenses"})
        {
            const Path root = project.Root / name;
            if (!std::filesystem::exists(root)) continue;
            std::vector<Path> pending{root};
            while (!pending.empty())
            {
                const auto current = pending.back();
                pending.pop_back();
                AssertNoLinks(current);
                if (IsReparsePoint(current))
                {
                    throw std::runtime_error("Linked resource rejected: " + WideToUtf8(current.wstring()));
                }
                const auto relative = std::filesystem::relative(current, project.Root).generic_u8string();
                if (relative.rfind("Assets/Cache/", 0) == 0 || relative == "Assets/Cache") continue;
                if (std::filesystem::is_directory(current))
                {
                    for (const auto& child : std::filesystem::directory_iterator(current))
                    {
                        pending.push_back(child.path());
                    }
                }
                else if (std::filesystem::is_regular_file(current))
                {
                    const auto size = std::filesystem::file_size(current);
                    if (size > 8ull * 1024 * 1024 * 1024)
                    {
                        throw std::runtime_error("Resource exceeds the 8 GiB entry limit.");
                    }
                    entries.push_back({relative, current, size});
                }
            }
        }
        if (entries.size() > 100000)
        {
            throw std::runtime_error("Resource pack entry count is invalid.");
        }
        std::uint64_t total = 0;
        for (const auto& entry : entries) total += entry.Size;
        if (total > 32ull * 1024 * 1024 * 1024) throw std::runtime_error("Resource pack size limit exceeded.");
        std::sort(entries.begin(), entries.end(), [](const Entry& left, const Entry& right)
        {
            return _stricmp(left.RelativePath.c_str(), right.RelativePath.c_str()) < 0;
        });
        std::filesystem::create_directories(pack.parent_path());
        std::ofstream output(pack, std::ios::binary | std::ios::trunc);
        if (!output) throw std::runtime_error("Cannot create resource pack.");
        output.write("GEPAK001", 8);
        AppendLittle32(output, static_cast<std::uint32_t>(entries.size()));
        std::array<char, 65536> buffer{};
        for (const auto& entry : entries)
        {
            if (entry.RelativePath.empty() || entry.RelativePath.size() > 4096 ||
                entry.RelativePath.find('\\') != std::string::npos)
            {
                throw std::runtime_error("Resource path is invalid.");
            }
            const auto hash = entry.Content ? HashBytes(*entry.Content) : HashFile(entry.SourcePath);
            AppendLittle32(output, static_cast<std::uint32_t>(entry.RelativePath.size()));
            AppendLittle64(output, entry.Size);
            AppendLittle(output, hash.Value.data(), hash.Value.size());
            output.write(entry.RelativePath.data(), static_cast<std::streamsize>(entry.RelativePath.size()));
            if (entry.Content)
            {
                output.write(entry.Content->data(), static_cast<std::streamsize>(entry.Content->size()));
            }
            else
            {
                std::ifstream input(entry.SourcePath, std::ios::binary);
                if (!input) throw std::runtime_error("Cannot read resource " + entry.RelativePath);
                std::uint64_t remaining = entry.Size;
                while (remaining)
                {
                    const auto count = static_cast<std::size_t>(std::min<std::uint64_t>(remaining, buffer.size()));
                    input.read(buffer.data(), static_cast<std::streamsize>(count));
                    if (input.gcount() != static_cast<std::streamsize>(count))
                    {
                        throw std::runtime_error("Resource changed during packaging: " + entry.RelativePath);
                    }
                    output.write(buffer.data(), static_cast<std::streamsize>(count));
                    remaining -= count;
                }
            }
            if (!output) throw std::runtime_error("Resource pack write failed.");
        }
        output.flush();
        if (!output) throw std::runtime_error("Resource pack write failed.");
    }

    struct PublishRecord
    {
        std::string RelativePath;
        std::uintmax_t Size = 0;
        std::string Hash;
        std::filesystem::path SourcePath;
    };

    std::string BuildManifest(const std::vector<PublishRecord>& records, const std::wstring& name)
    {
        std::ostringstream stream;
        stream << "{\"schemaVersion\":1,\"name\":\"" << JsonEscape(WideToUtf8(name)) << "\",\"files\":[";
        for (std::size_t index = 0; index < records.size(); ++index)
        {
            if (index) stream << ',';
            stream << "{\"path\":\"" << JsonEscape(records[index].RelativePath) << "\",\"size\":"
                << records[index].Size << ",\"sha256\":\"" << records[index].Hash << "\"}";
        }
        stream << "]}\n";
        return stream.str();
    }

    std::vector<std::pair<std::string, std::string>> ParseManifestRecords(const std::string& manifest)
    {
        static const std::regex pattern("\\{\"path\":\"([^\"]+)\",\"size\":[0-9]+,\"sha256\":\"([0-9a-f]{64})\"\\}");
        std::vector<std::pair<std::string, std::string>> records;
        for (std::sregex_iterator iterator(manifest.begin(), manifest.end(), pattern); iterator != std::sregex_iterator(); ++iterator)
        {
            records.emplace_back((*iterator)[1].str(), (*iterator)[2].str());
        }
        return records;
    }

    std::string PublishProject(const Path& nativeRoot, const Path& pack, const std::wstring& name,
        const Path& output)
    {
        AssertNoLinks(output);
        if (output.parent_path().empty())
        {
            throw std::runtime_error("Cannot publish to a filesystem root.");
        }
        AssertNoLinks(output.parent_path());
        std::vector<PublishRecord> records;
        std::map<std::string, Path> sources;
        const auto add = [&](const std::string& relativePath, const Path& source)
        {
            AssertNoLinks(source);
            if (!std::filesystem::exists(source)) throw std::runtime_error("Missing publication input: " + WideToUtf8(source.wstring()));
            if (relativePath.empty() || relativePath.find('\\') != std::string::npos ||
                relativePath.find("..") != std::string::npos)
            {
                throw std::runtime_error("Invalid publication path: " + relativePath);
            }
            records.push_back({relativePath, std::filesystem::file_size(source), Hex(HashFile(source)), source});
            sources[relativePath] = source;
        };

        add(WideToUtf8(name + L".exe"), nativeRoot / (name + L".exe"));
        const auto runtimeRoot = nativeRoot / L"Runtime";
        if (!std::filesystem::exists(runtimeRoot))
        {
            throw std::runtime_error("Project runtime directory is missing.");
        }
        for (const auto& item : std::filesystem::recursive_directory_iterator(runtimeRoot))
        {
            if (!item.is_regular_file() || IsReparsePoint(item.path())) continue;
            const auto relative = std::filesystem::relative(item.path(), runtimeRoot);
            add("Runtime/" + relative.generic_u8string(), item.path());
        }
        if (!std::filesystem::exists(pack))
            {
            throw std::runtime_error("Resource pack is missing.");
        }
        add("Data/" + WideToUtf8(name + L".gpak"), pack);
        if (std::filesystem::exists(output / L"Config" / L".runtime.lock"))
            throw std::runtime_error("Close the running game before replacing its package.");
        const auto existingConfig = output / L"Config" / L"Config.ini";
        add("Config/Config.ini", std::filesystem::exists(existingConfig) ? existingConfig : pack.parent_path() / L"Config.ini");
        const auto mutablePath = [](const std::string& path)
        { return path.rfind("Config/", 0) == 0 || path.rfind("Cache/", 0) == 0 ||
            path.rfind("Logs/", 0) == 0 || path.rfind("UserData/", 0) == 0; };

        std::sort(records.begin(), records.end(), [](const auto& left, const auto& right)
        {
            return _stricmp(left.RelativePath.c_str(), right.RelativePath.c_str()) < 0;
        });
        const auto manifest = BuildManifest(records, name);
        const auto marker = output / L"Metadata" / L".gengine-output.json";
        std::map<std::string, std::string> previous;
        if (std::filesystem::exists(output))
        {
            bool hasManagedContent = false;
            for (const auto& item : std::filesystem::directory_iterator(output))
            {
                hasManagedContent = true;
                if (IsReparsePoint(item.path()))
                {
                    throw std::runtime_error("Output contains links.");
                }
                const auto filename = WideToUtf8(item.path().filename().wstring());
                if (item.is_directory() && filename != "Runtime" && filename != "Data" && filename != "Metadata" &&
                    filename != "Config" && filename != "Cache" && filename != "Logs" && filename != "UserData")
                {
                    throw std::runtime_error("Output contains unmanaged directories; refusing replacement.");
                }
                if (item.is_regular_file() && _wcsicmp(Path(item.path()).extension().c_str(), L".exe") != 0)
                {
                    throw std::runtime_error("Output contains unmanaged files; refusing replacement.");
                }
            }
            if (hasManagedContent && !std::filesystem::exists(marker))
            {
                throw std::runtime_error("Output contains unmanaged files; choose an empty folder.");
            }
            if (std::filesystem::exists(marker))
            {
                const auto oldManifest = ReadText(marker);
                const auto oldRecords = ParseManifestRecords(oldManifest);
                if (oldRecords.empty()) throw std::runtime_error("Unknown output manifest version.");
                for (const auto& record : oldRecords)
                    if (!mutablePath(record.first) || std::filesystem::exists(output / Utf8ToWide(record.first)))
                        previous[record.first] = record.second;
                for (const auto& item : std::filesystem::recursive_directory_iterator(output))
                {
                    if (IsReparsePoint(item.path())) throw std::runtime_error("Linked output state rejected.");
                    if (!item.is_regular_file()) continue;
                    const auto relative = std::filesystem::relative(item.path(), output).generic_u8string();
                    if (relative != "Metadata/.gengine-output.json" && !previous.count(relative) && !mutablePath(relative))
                    {
                        throw std::runtime_error("Output contains unmanaged files; refusing replacement.");
                    }
                }
                for (const auto& record : oldRecords)
                {
                    const auto path = output / Utf8ToWide(record.first);
                    if (!mutablePath(record.first) && (!std::filesystem::exists(path) || Hex(HashFile(path)) != record.second))
                    {
                        throw std::runtime_error("Published output was modified; choose a new destination.");
                    }
                }
                if (oldManifest == manifest)
                {
                    return "{\"output\":\"" + JsonEscape(WideToUtf8(output.wstring())) + "\",\"executable\":\"" +
                        JsonEscape(WideToUtf8((output / (name + L".exe")).wstring())) + "\",\"resourcePack\":\"" +
                        JsonEscape(WideToUtf8((output / L"Data" / (name + L".gpak")).wstring())) +
                        "\",\"copyFiles\":0}\n";
                }
            }
        }
        const auto parent = output.parent_path();
        std::filesystem::create_directories(parent);
        OutputLock lock(parent, output);
        const auto staging = parent / (L".gengine-publish-" + RandomId());
        const auto backup = output.wstring() + L".previous-" + RandomId();
        std::uint64_t copied = 0;
        try
        {
            std::filesystem::create_directories(staging);
            for (const auto& record : records)
            {
                const auto target = staging / Utf8ToWide(record.RelativePath);
                std::filesystem::create_directories(target.parent_path());
                const auto old = previous.find(record.RelativePath);
                if (old != previous.end() && old->second == record.Hash && !mutablePath(record.RelativePath))
                {
                const auto oldPath = output / Utf8ToWide(record.RelativePath);
                    if (!CreateHardLinkW(target.c_str(), oldPath.c_str(), nullptr))
                    {
                        throw std::runtime_error("Cannot reuse unchanged publication file.");
                    }
                }
                else
                {
                    std::filesystem::copy_file(record.SourcePath, target, std::filesystem::copy_options::none);
                    ++copied;
                }
                if (Hex(HashFile(target)) != record.Hash)
                {
                    throw std::runtime_error("Publish integrity check failed.");
                }
            }
            WriteText(staging / L"Metadata" / L".gengine-output.json", manifest);
            for (const auto* folder : {L"Config", L"Cache", L"Logs", L"UserData"})
            {
                const auto source = output / folder;
                if (!std::filesystem::exists(source)) continue;
                for (const auto& item : std::filesystem::recursive_directory_iterator(source))
                {
                    AssertNoLinks(item.path());
                    const auto relative = std::filesystem::relative(item.path(), output);
                    if (relative.generic_wstring() == L"Config/Config.ini") continue;
                    const auto target = staging / relative;
                    if (item.is_directory()) std::filesystem::create_directories(target);
                    else if (item.is_regular_file())
                    { std::filesystem::create_directories(target.parent_path()); std::filesystem::copy_file(item.path(), target); }
                }
            }
            bool movedOld = false;
            if (std::filesystem::exists(output))
            {
                std::filesystem::rename(output, backup);
                movedOld = true;
            }
            try
            {
                std::filesystem::rename(staging, output);
            }
            catch (...)
            {
                if (movedOld && !std::filesystem::exists(output))
                {
                    std::filesystem::rename(backup, output);
                }
                throw;
            }
        }
        catch (...)
        {
            if (std::filesystem::exists(staging)) RemoveTree(staging, parent, L".gengine-publish-");
            throw;
        }
        return "{\"output\":\"" + JsonEscape(WideToUtf8(output.wstring())) + "\",\"executable\":\"" +
            JsonEscape(WideToUtf8((output / (name + L".exe")).wstring())) + "\",\"resourcePack\":\"" +
            JsonEscape(WideToUtf8((output / L"Data" / (name + L".gpak")).wstring())) + "\",\"copyFiles\":" +
            std::to_string(copied) + ",\"previousOutput\":\"" + JsonEscape(WideToUtf8(backup)) + "\"}\n";
    }

    void BuildMonoSdk(const Path& repoRoot, bool force)
    {
        const auto build = repoRoot / L"build" / L"mono" / L"windows-x86_64" / L"6.10.0.87";
        const auto install = repoRoot / L"dist" / L"mono" / L"windows-x86_64" / L"6.10.0.87";
        const auto sdk = install / L"Mono";
        if (force && std::filesystem::exists(install)) RemoveTree(install, repoRoot, L"dist\\mono\\");
        if (std::filesystem::exists(sdk / L"include" / L"mono-2.0"))
        {
            std::cout << "Mono SDK already installed: " << WideToUtf8(sdk.wstring()) << '\n';
            return;
        }
        std::filesystem::create_directories(build);
        std::filesystem::create_directories(install);
        const auto msi = build / L"mono-6.10.0.87-x64-0.msi";
        static constexpr char expected[] = "8853a0f1caa495ce49d0f61ba72c904d0d1dac0c1391949157dc7182ccd9d0fe4";
        if (!std::filesystem::exists(msi) || Hex(HashFile(msi)) != expected)
        {
            const auto curl = SearchTool(L"curl.exe");
            std::cout << "Downloading Mono SDK installer\n";
            auto result = RunProcess(curl, {L"--ssl-no-revoke", L"-L", L"--retry", L"3", L"--output",
                msi.wstring(), L"https://download.mono-project.com/archive/6.10.0/windows-installer/mono-6.10.0.87-x64-0.msi"},
                repoRoot);
            if (result.ExitCode != 0) throw std::runtime_error("Failed to download Mono SDK installer.");
            if (Hex(HashFile(msi)) != expected) throw std::runtime_error("Mono SDK hash mismatch.");
        }
        std::cout << "Extracting Mono SDK\n";
        auto extraction = RunProcess(L"msiexec.exe", {L"/a", msi.wstring(), L"/qn",
            L"TARGETDIR=" + install.wstring()}, repoRoot);
        if (extraction.ExitCode != 0)
        {
            throw std::runtime_error("Mono SDK administrative extraction failed.");
        }
        if (!std::filesystem::exists(sdk / L"lib" / L"libmono-static-sgen.lib"))
        {
            throw std::runtime_error("Mono SDK extraction did not produce libmono-static-sgen.lib.");
        }
        const auto curl = SearchTool(L"curl.exe");
        for (const auto& document : {std::make_pair(std::wstring(L"LICENSE"),
            std::wstring(L"https://raw.githubusercontent.com/mono/mono/mono-6.10.0.87/LICENSE")),
            std::make_pair(std::wstring(L"PATENTS.TXT"),
            std::wstring(L"https://raw.githubusercontent.com/mono/mono/mono-6.10.0.87/PATENTS.TXT"))})
        {
            auto result = RunProcess(curl, {L"--ssl-no-revoke", L"-L", L"--silent", L"--output",
                (sdk / document.first).wstring(), document.second}, repoRoot);
            if (result.ExitCode != 0) throw std::runtime_error("Failed to download Mono notices.");
        }
        WriteText(install / L"mono-sdk.json", "{\"Version\":\"6.10.0.87\",\"Source\":\"https://download.mono-project.com/archive/6.10.0/windows-installer/mono-6.10.0.87-x64-0.msi\",\"Sha256\":\"" + std::string(expected) + "\",\"SdkRoot\":\"" + JsonEscape(WideToUtf8(sdk.wstring())) + "\"}");
        std::cout << "Mono SDK installed: " << WideToUtf8(sdk.wstring()) << '\n';
    }

    void BuildQtSdk(const Path& repoRoot, const std::wstring& configuration, int jobs,
        const std::wstring& python, bool configureOnly)
    {
        const auto source = repoRoot / L"GEngine" / L"GEngine-ThirdParty" / L"qtbase";
        if (!std::filesystem::exists(source / L"CMakeLists.txt"))
        {
            throw std::runtime_error("Initialize the qtbase submodule first.");
        }
        const auto build = repoRoot / L"build" / L"qt6" / L"windows-x86_64" / configuration;
        const auto install = repoRoot / L"dist" / L"qt6" / L"windows-x86_64" / configuration;
        const auto visualStudio = FindVisualStudio();
        const auto cmake = FindCMake().wstring();
        const auto ninja = (visualStudio / L"Common7" / L"IDE" / L"CommonExtensions" / L"Microsoft" / L"CMake" /
            L"Ninja" / L"ninja.exe").wstring();
        Path requestedPython(python);
        if (!requestedPython.is_absolute() || !std::filesystem::exists(requestedPython))
        {
            requestedPython = Path(SearchTool((python + L".exe")));
        }
        const auto pythonPath = requestedPython.wstring();
        std::filesystem::create_directories(build);
        RunVcLogged(visualStudio, cmake, {L"-S", source.wstring(), L"-B", build.wstring(), L"-G", L"Ninja",
            L"-DCMAKE_MAKE_PROGRAM=" + ninja, L"-DCMAKE_BUILD_TYPE=" + configuration,
            L"-DCMAKE_INSTALL_PREFIX=" + install.wstring(), L"-DPython3_EXECUTABLE=" + pythonPath,
            L"-DQT_BUILD_TESTS=OFF", L"-DQT_BUILD_EXAMPLES=OFF", L"-DFEATURE_opengl=ON",
            L"-DFEATURE_vulkan=OFF", L"-DFEATURE_sql=OFF", L"-DFEATURE_network=OFF", L"-DFEATURE_dbus=OFF",
            L"-DFEATURE_printsupport=OFF", L"-DFEATURE_testlib=OFF", L"-DFEATURE_concurrent=OFF"},
            repoRoot, build / L"configure.log");
        if (configureOnly) return;
        RunVcLogged(visualStudio, cmake, {L"--build", build.wstring(), L"--parallel",
            std::to_wstring(jobs)}, repoRoot, build / L"build.log");
        RunVcLogged(visualStudio, cmake, {L"--install", build.wstring()}, repoRoot, build / L"install.log");
        std::cout << "Qt SDK installed: " << WideToUtf8(install.wstring()) << '\n';
    }

    void CopyDirectory(const Path& source, const Path& destination)
    {
        std::filesystem::create_directories(destination);
        for (const auto& item : std::filesystem::recursive_directory_iterator(source))
        {
            const auto target = destination / std::filesystem::relative(item.path(), source);
            if (item.is_directory()) std::filesystem::create_directories(target);
            else if (item.is_regular_file())
            {
                std::filesystem::copy_file(item.path(), target, std::filesystem::copy_options::overwrite_existing);
            }
        }
    }

    void BuildQtEditor(const Path& repoRoot, const std::wstring& configuration, int jobs,
        const std::wstring& qtSdk, bool premake, bool protocolOnly, bool test)
    {
        if (!protocolOnly) BuildMonoSdk(repoRoot, false);
        const auto sdk = qtSdk.empty() ? repoRoot / L"dist" / L"qt6" / L"windows-x86_64" / configuration : Path(qtSdk);
        auto build = repoRoot / L"build" / L"qt-editor" / L"windows-x86_64" / configuration;
        if (premake) build = repoRoot / L"build" / L"premake-vs2022" / L"windows-x86_64" / configuration / L"qt-editor-logs";
        if (protocolOnly) build += L"-protocol";
        std::filesystem::create_directories(build);
        const auto visualStudio = FindVisualStudio();
        const auto cmake = FindCMake().wstring();
        if (premake)
        {
            if (protocolOnly) throw std::runtime_error("Use CMake protocol-only mode.");
            const auto premakeTool = repoRoot / L"BuildScripts" / L"Premake" / L"premake5.exe";
            RunVcLogged(visualStudio, premakeTool.wstring(), {std::wstring(L"--qt6-sdk=") +
                sdk.parent_path().wstring(), L"vs2022"}, repoRoot, build / L"generate.log");
            const auto msbuild = visualStudio / L"MSBuild" / L"Current" / L"Bin" / L"MSBuild.exe";
            RunVcLogged(visualStudio, msbuild.wstring(), {(repoRoot / L"BuildScripts" / L"Premake" /
                L"GEngine-EditorEngineWorker.vcxproj").wstring(), std::wstring(L"/p:Configuration=") + configuration,
                L"/p:Platform=x64", L"/m:4", L"/nologo", L"/v:minimal"}, repoRoot,
                build / L"engine-worker-build.log");
            for (const auto& target : {L"GEngineTools", L"GEngine-EditorDiagnosticWorker",
                L"GEngine-EditorHost", L"GEngine-EditorProtocolTests"})
            {
                RunVcLogged(visualStudio, msbuild.wstring(), {(repoRoot / L"BuildScripts" / L"Premake" /
                    (std::wstring(target) + L".vcxproj")).wstring(), std::wstring(L"/p:Configuration=") + configuration,
                    L"/p:Platform=x64", L"/m:4", L"/nologo", L"/v:minimal"}, repoRoot,
                    build / (std::wstring(target) + L"-build.log"));
            }
            if (!test) return;
            const auto outputRoot = repoRoot / L"dist" / L"premake-vs2022" / L"windows-x86_64" / configuration;
            RunLogged((outputRoot / L"GEngine-EditorProtocolTests" / L"GEngine-EditorProtocolTests.exe").wstring(),
                {}, repoRoot, build / L"protocol-tests.log");
            RunLogged((outputRoot / L"GEngine-EditorHost" / L"GEngine-EditorHost.exe").wstring(),
                {L"--self-test"}, repoRoot, build / L"recovery-tests.log", true);
            RunLogged((outputRoot / L"GEngine-EditorHost" / L"GEngine-EditorHost.exe").wstring(),
                {L"--packaging-self-test"}, repoRoot, build / L"packaging-tests.log", true);
            RunLogged((outputRoot / L"GEngine-EditorHost" / L"GEngine-EditorHost.exe").wstring(),
                {L"--self-test", L"--engine-check", L"--worker", (outputRoot / L"GEngine-EditorEngineWorker" /
                L"GEngine-EditorEngineWorker.exe").wstring()}, repoRoot, build / L"engine-tests.log", true);
            return;
        }
        const auto protocolFlag = protocolOnly ? L"ON" : L"OFF";
        const auto editorSource = repoRoot / L"BuildScripts" / L"Editor";
        const auto ninja = (visualStudio / L"Common7" / L"IDE" / L"CommonExtensions" / L"Microsoft" / L"CMake" /
            L"Ninja" / L"ninja.exe").wstring();
        RunVcLogged(visualStudio, cmake, {L"-S", editorSource.wstring(), L"-B", build.wstring(), L"-G", L"Ninja",
            std::wstring(L"-DCMAKE_MAKE_PROGRAM=") + ninja, std::wstring(L"-DCMAKE_BUILD_TYPE=") + configuration,
            std::wstring(L"-DCMAKE_PREFIX_PATH=") + sdk.wstring(), std::wstring(L"-DGE_EDITOR_PROTOCOL_ONLY=") +
            protocolFlag},
            repoRoot, build / L"configure.log");
        RunVcLogged(visualStudio, cmake, {L"--build", build.wstring(), L"--parallel",
            std::to_wstring(jobs)}, repoRoot, build / L"build.log");
        if (!protocolOnly)
        {
            const auto engineBuild = repoRoot / L"build" / L"cmake-vs2022" / L"windows-x86_64";
            RunVcLogged(visualStudio, cmake, {L"-S", (repoRoot / L"BuildScripts" / L"CMake").wstring(),
                L"-B", engineBuild.wstring(), L"-G", L"Visual Studio 17 2022", L"-A", L"x64",
                L"-DGE_TOOL_ID=cmake-vs2022"}, repoRoot, build / L"engine-configure.log");
            RunVcLogged(visualStudio, cmake, {L"--build", engineBuild.wstring(), L"--config", configuration,
                L"--target", L"GEngine-EditorEngineWorker", L"--parallel", std::to_wstring(jobs)},
                repoRoot, build / L"engine-build.log");
            const auto engineOutput = repoRoot / L"dist" / L"cmake-vs2022" / L"windows-x86_64" / configuration /
                L"GEngine-EditorEngineWorker";
            const auto editorOutput = repoRoot / L"dist" / L"qt-editor" / L"windows-x86_64" / configuration /
                L"GEngine-EditorEngineWorker";
            CopyDirectory(engineOutput, editorOutput);
        }
        if (!test) return;
        const auto ctest = FindCMake().parent_path() / L"ctest.exe";
        RunVcLogged(visualStudio, ctest.wstring(), {L"--test-dir", build.wstring(), L"--output-on-failure",
            L"-C", configuration}, repoRoot, build / L"tests.log");
        if (!protocolOnly)
        {
            const auto outputRoot = repoRoot / L"dist" / L"qt-editor" / L"windows-x86_64" / configuration;
            RunLogged((outputRoot / L"GEngine-EditorHost" / L"GEngine-EditorHost.exe").wstring(),
                {L"--self-test", L"--engine-check", L"--worker", (outputRoot / L"GEngine-EditorEngineWorker" /
                L"GEngine-EditorEngineWorker.exe").wstring()}, repoRoot, build / L"engine-tests.log", true);
        }
    }

    int RunQtEditor(const std::wstring& configuration, const std::wstring& generator, bool printPath)
    {
        const auto root = RepoRoot();
        std::vector<Path> candidates;
        const auto addCandidate = [&](const std::wstring& tool)
        {
            const auto path = root / L"dist" / tool / L"windows-x86_64" / configuration /
                L"GEngine-EditorHost" / L"GEngine-EditorHost.exe";
            if (std::filesystem::exists(path)) candidates.push_back(path);
        };
        if (generator == L"CMake" || generator == L"Auto") addCandidate(L"qt-editor");
        if (generator == L"Premake" || generator == L"Auto") addCandidate(L"premake-vs2022");
        if (candidates.empty())
        {
            std::cerr << "No " << WideToUtf8(configuration) << " editor build found for " << WideToUtf8(generator)
                << ". Run GEngineTools qt-editor first.\n";
            return 1;
        }
        std::sort(candidates.begin(), candidates.end(), [](const Path& left, const Path& right)
        {
            return std::filesystem::last_write_time(left) > std::filesystem::last_write_time(right);
        });
        if (printPath)
        {
            std::cout << WideToUtf8(candidates.front().wstring()) << '\n';
            return 0;
        }
        const auto result = RunProcess(candidates.front().wstring(), {}, candidates.front().parent_path());
        return result.ExitCode;
    }

    void ExportProject(const Path& projectPath, const Path& outputPath,
        const std::wstring& executableName, const std::wstring& iconPath)
    {
        const auto project = LoadProject(projectPath);
        ExportOptions options;
        options.ExecutableName = Trim(executableName);
        if (options.ExecutableName.empty()) options.ExecutableName = project.Name;
        if (!ValidProjectName(options.ExecutableName))
        {
            throw std::runtime_error("Invalid executable name.");
        }
        if (!iconPath.empty())
        {
            options.Icon = std::filesystem::absolute(Path(iconPath));
            AssertNoLinks(options.Icon);
            if (!std::filesystem::is_regular_file(options.Icon) ||
                _wcsicmp(options.Icon.extension().c_str(), L".ico") != 0 ||
                std::filesystem::file_size(options.Icon) > 64ull * 1024 * 1024)
            {
                throw std::runtime_error("Choose a readable .ico icon.");
            }
        }
        const auto output = std::filesystem::absolute(outputPath);
        if (Within(output, project.Root) || Within(project.Root, output))
        {
            throw std::runtime_error("Choose an output folder outside the project.");
        }
        ProcessJob job;
        ProjectMutex mutex(project.Project);
        mutex.RequireOwned();
        std::cout << "Compiling project and engine (Release)...\n";
        const auto nativeRoot = BuildProject(RepoRoot(), project, options);
        const auto staging = TemporaryPath() / (L"gengine-package-" + RandomId());
        std::filesystem::create_directories(staging);
        const auto pack = staging / (options.ExecutableName + L".gpak");
        std::cout << "Creating resource pack...\n";
        WriteResourcePack(project, pack);
        const auto sourceConfig = project.Root / L"Config.ini";
        if (std::filesystem::exists(sourceConfig))
        {
            AssertNoLinks(sourceConfig);
            if (!std::filesystem::is_regular_file(sourceConfig) || std::filesystem::file_size(sourceConfig) > 1024 * 1024)
                throw std::runtime_error("Project Config.ini is invalid or exceeds 1 MiB.");
            std::filesystem::copy_file(sourceConfig, staging / L"Config.ini");
        }
        else WriteText(staging / L"Config.ini", "GraphicsAPI: 3\nFramesInFlight: 2\nVSync: 1\n"
            "CommandBufferCount: 8\nShaderCacheDirectory: Cache/Shaders/\n"
            "LongPressThresholdMs: 500\nDoubleClickThresholdMs: 500\nWindowManagerAPI: 0\n");
        std::cout << "Publishing executable and resource pack...\n";
        const auto result = PublishProject(nativeRoot, pack, options.ExecutableName, output);
        RemoveTree(staging, TemporaryPath(), L"gengine-package-");
        std::cout << result;
    }

    struct Options
    {
        std::map<std::wstring, std::wstring> Values;
        std::set<std::wstring> Flags;
        std::vector<std::wstring> Positional;
    };

    Options ParseArguments(const std::vector<std::wstring>& arguments)
    {
        Options options;
        for (std::size_t index = 0; index < arguments.size(); ++index)
        {
            std::wstring argument = arguments[index];
            if (!argument.empty() && (argument.front() == L'-' || argument.front() == L'/'))
            {
                while (!argument.empty() && (argument.front() == L'-' || argument.front() == L'/')) argument.erase(0, 1);
                const auto equals = argument.find(L'=');
                if (equals != std::wstring::npos)
                {
                    options.Values[argument.substr(0, equals)] = argument.substr(equals + 1);
                }
                else
                {
                    auto key = argument;
                    std::transform(key.begin(), key.end(), key.begin(), [](wchar_t value)
                    {
                        return static_cast<wchar_t>(towlower(value));
                    });
                    if (index + 1 < arguments.size() && arguments[index + 1].front() != L'-' &&
                        arguments[index + 1].front() != L'/')
                    {
                        options.Values[key] = arguments[++index];
                    }
                    else options.Flags.insert(key);
                }
            }
            else options.Positional.push_back(argument);
        }
        return options;
    }

    std::wstring RequiredValue(const Options& options, const std::wstring& key)
    {
        const auto found = options.Values.find(key);
        if (found == options.Values.end() || found->second.empty())
        {
            throw std::runtime_error("Missing required --" + WideToUtf8(key) + " value.");
        }
        return found->second;
    }

    std::wstring OptionalValue(const Options& options, const std::wstring& key)
    {
        const auto found = options.Values.find(key);
        return found == options.Values.end() ? std::wstring{} : found->second;
    }

    int Dispatch(const std::vector<std::wstring>& arguments)
    {
        if (arguments.empty())
        {
            std::cerr << "Usage: GEngineTools <mono-sdk|qt-sdk|qt-editor|run-qt-editor|export-project> [options]\n";
            return 2;
        }
        const auto command = arguments.front();
        const auto options = ParseArguments(std::vector<std::wstring>(arguments.begin() + 1, arguments.end()));
        const auto root = RepoRoot();
        if (command == L"mono-sdk")
        {
            BuildMonoSdk(root, options.Flags.count(L"force") != 0);
            return 0;
        }
        if (command == L"qt-sdk")
        {
            const auto configuration = options.Values.count(L"configuration") ? options.Values.at(L"configuration") : L"Release";
            const auto jobs = options.Values.count(L"jobs") ? std::stoi(options.Values.at(L"jobs")) : 6;
            const auto python = options.Values.count(L"python") ? options.Values.at(L"python") : L"python";
            BuildQtSdk(root, configuration, jobs, python, options.Flags.count(L"configure-only") != 0);
            return 0;
        }
        if (command == L"qt-editor")
        {
            const auto configuration = options.Values.count(L"configuration") ? options.Values.at(L"configuration") : L"Release";
            const auto jobs = options.Values.count(L"jobs") ? std::stoi(options.Values.at(L"jobs")) : 6;
            const auto qtSdk = options.Values.count(L"qt-sdk") ? options.Values.at(L"qt-sdk") : L"";
            BuildQtEditor(root, configuration, jobs, qtSdk, options.Flags.count(L"premake") != 0,
                options.Flags.count(L"protocol-only") != 0, options.Flags.count(L"test") != 0);
            return 0;
        }
        if (command == L"run-qt-editor")
        {
            const auto configuration = options.Positional.empty() ? L"Release" : options.Positional.front();
            const auto generator = options.Positional.size() > 1 ? options.Positional[1] : L"Auto";
            return RunQtEditor(configuration, generator, options.Flags.count(L"print-path") != 0 ||
                options.Flags.count(L"printpath") != 0);
        }
        if (command == L"export-project")
        {
            ExportProject(Path(RequiredValue(options, L"project")), Path(RequiredValue(options, L"output")),
                OptionalValue(options, L"executable-name"), OptionalValue(options, L"icon"));
            return 0;
        }
        std::cerr << "Unknown command: " << WideToUtf8(command) << '\n';
        return 2;
    }
}

int main()
{
    try
    {
        int argumentCount = 0;
        const auto argumentsPointer = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
        if (!argumentsPointer) return 2;
        std::vector<std::wstring> arguments;
        for (int index = 1; index < argumentCount; ++index) arguments.push_back(argumentsPointer[index]);
        LocalFree(argumentsPointer);
        return Dispatch(arguments);
    }
    catch (const std::exception& error)
    {
        std::cerr << "GEngineTools: " << error.what() << '\n';
        return 1;
    }
}
