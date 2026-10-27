#ifndef GE_RESOURCE_PACK_TEST
#include "GEpch.h"
#endif
#include "GEngine/Runtime/ResourcePack.h"
#include <Windows.h>
#include <bcrypt.h>
#include <array>
#include <fstream>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <cwctype>
#include <map>
#include <mutex>
#include <memory>

namespace GEngine
{
    namespace
    {
        void Read(std::istream& stream, void* bytes, std::size_t count)
        { if (!stream.read(static_cast<char*>(bytes), static_cast<std::streamsize>(count))) throw std::runtime_error("Truncated resource pack."); }
        std::uint64_t Number(std::istream& stream, unsigned bytes)
        {
            std::uint64_t result = 0;
            for (unsigned i = 0; i < bytes; ++i) { unsigned char value; Read(stream, &value, 1); result |= std::uint64_t(value) << (i * 8); }
            return result;
        }
        struct Hash
        {
            BCRYPT_ALG_HANDLE Algorithm = nullptr;
            BCRYPT_HASH_HANDLE Handle = nullptr;
            Hash()
            {
                if (BCryptOpenAlgorithmProvider(&Algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0) throw std::runtime_error("SHA-256 unavailable.");
                if (BCryptCreateHash(Algorithm, &Handle, nullptr, 0, nullptr, 0, 0) < 0)
                { BCryptCloseAlgorithmProvider(Algorithm, 0); Algorithm = nullptr; throw std::runtime_error("SHA-256 initialization failed."); }
            }
            ~Hash() { if (Handle) BCryptDestroyHash(Handle); if (Algorithm) BCryptCloseAlgorithmProvider(Algorithm, 0); }
            void Append(unsigned char* bytes, ULONG count)
            { if (BCryptHashData(Handle, bytes, count, 0) < 0) throw std::runtime_error("SHA-256 update failed."); }
            std::array<unsigned char, 32> Finish()
            {
                std::array<unsigned char, 32> value{};
                if (BCryptFinishHash(Handle, value.data(), static_cast<ULONG>(value.size()), 0) < 0) throw std::runtime_error("SHA-256 finalization failed.");
                return value;
            }
        };
        std::filesystem::path SafePath(const std::string& utf8)
        {
            if (utf8.empty() || utf8.front() == '/' || utf8.back() == '/' || utf8.find("//") != std::string::npos ||
                utf8.find('\\') != std::string::npos) throw std::runtime_error("Invalid resource path.");
            auto path = std::filesystem::u8path(utf8);
            if (path.is_absolute() || path.has_root_path()) throw std::runtime_error("Absolute resource path rejected.");
            for (const auto& part : path)
            {
                const auto value = part.wstring();
                if (value.empty() || value == L"." || value == L".." || value.back() == L'.' || value.back() == L' ' ||
                    value.find_first_of(L"<>:\"|?*") != std::wstring::npos ||
                    std::any_of(value.begin(), value.end(), [](wchar_t c) { return c < 32; }))
                    throw std::runtime_error("Unsafe resource path rejected.");
                auto stem = value.substr(0, value.find(L'.'));
                for (auto& c : stem) c = static_cast<wchar_t>(towupper(c));
                if (stem == L"CON" || stem == L"PRN" || stem == L"AUX" || stem == L"NUL" ||
                    (stem.size() == 4 && (stem.substr(0, 3) == L"COM" || stem.substr(0, 3) == L"LPT") && stem[3] >= L'1' && stem[3] <= L'9'))
                    throw std::runtime_error("Reserved resource filename rejected.");
            }
            return path;
        }
        struct WindowsLess
        {
            bool operator()(const std::wstring& a, const std::wstring& b) const
            { return CompareStringOrdinal(a.data(), static_cast<int>(a.size()), b.data(), static_cast<int>(b.size()), TRUE) == CSTR_LESS_THAN; }
        };
        struct Entry { std::uint64_t Offset, Size; std::array<unsigned char, 32> Digest; };
        struct Archive
        {
            HANDLE Lock = INVALID_HANDLE_VALUE;
            HANDLE StateLock = nullptr;
            std::ifstream Input;
            std::map<std::wstring, Entry, WindowsLess> Entries;
            std::mutex Mutex;
            ~Archive() { Input.close(); if (Lock != INVALID_HANDLE_VALUE) CloseHandle(Lock); if (StateLock) CloseHandle(StateLock); }
        };
        std::shared_ptr<Archive> Mounted;
        std::mutex MountMutex;
        std::shared_ptr<Archive> Snapshot()
        { std::lock_guard<std::mutex> guard(MountMutex); return Mounted; }
        std::filesystem::path LogicalPath(const std::filesystem::path& path)
        {
            auto relative = path;
            if (path.is_absolute()) relative = path.lexically_relative(std::filesystem::current_path());
            for (const auto& part : relative)
                if (part == L"..") throw std::runtime_error("Resource path escapes the session.");
            relative = relative.lexically_normal();
            if (relative.filename().empty()) relative = relative.parent_path();
            return SafePath(relative.generic_u8string());
        }
        bool Writable(const std::filesystem::path& path)
        {
            auto name = path.generic_wstring();
            std::transform(name.begin(), name.end(), name.begin(), towlower);
            return name == L"config.ini" || name == L"config" || name.rfind(L"config/", 0) == 0 ||
                name == L"logs" || name.rfind(L"logs/", 0) == 0 ||
                name == L"cache" || name.rfind(L"cache/", 0) == 0 ||
                name == L"userdata" || name.rfind(L"userdata/", 0) == 0;
        }
    }
    void MountResourcePack(const std::filesystem::path& pack)
    {
        auto archive = std::make_shared<Archive>();
        archive->Lock = CreateFileW(pack.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (archive->Lock == INVALID_HANDLE_VALUE) throw std::runtime_error("Cannot open immutable resource pack.");
        archive->Input.open(pack, std::ios::binary);
        auto& input = archive->Input;
        std::array<char, 8> magic{}; Read(input, magic.data(), magic.size());
        if (std::string(magic.data(), magic.size()) != "GEPAK001") throw std::runtime_error("Unsupported resource pack version.");
        const auto count = Number(input, 4);
        if (count > 100000) throw std::runtime_error("Invalid resource count.");
        std::set<std::wstring, WindowsLess> paths;
        const auto packSize = std::filesystem::file_size(pack);
        std::uint64_t total = 0;
        for (std::uint64_t i = 0; i < count; ++i)
        {
            const auto pathLength = Number(input, 4), size = Number(input, 8);
            std::array<unsigned char, 32> expected{}; Read(input, expected.data(), expected.size());
            if (!pathLength || pathLength > 4096 || size > 8ull * 1024 * 1024 * 1024 || total + size > 32ull * 1024 * 1024 * 1024)
                throw std::runtime_error("Resource pack limits exceeded.");
            total += size;
            std::string utf8(static_cast<std::size_t>(pathLength), '\0'); Read(input, utf8.data(), utf8.size());
            const auto relative = SafePath(utf8);
            if (!paths.insert(relative.generic_wstring()).second) throw std::runtime_error("Duplicate resource path.");
            const auto position = static_cast<std::uint64_t>(input.tellg());
            if (position > packSize || size > packSize - position) throw std::runtime_error("Truncated resource payload.");
            archive->Entries.emplace(relative.generic_wstring(), Entry{position, size, expected});
            Hash hash;
            std::array<unsigned char, 65536> buffer{};
            for (std::uint64_t remaining = size; remaining;)
            {
                const auto bytes = static_cast<std::size_t>((std::min)(remaining, static_cast<std::uint64_t>(buffer.size())));
                Read(input, buffer.data(), bytes); hash.Append(buffer.data(), static_cast<ULONG>(bytes));
                remaining -= bytes;
            }
            if (hash.Finish() != expected) throw std::runtime_error("Resource checksum mismatch.");
        }
        if (input.peek() != std::char_traits<char>::eof()) throw std::runtime_error("Unexpected trailing resource data.");
        for (const auto& item : archive->Entries)
        {
            for (auto parent = std::filesystem::path(item.first).parent_path(); !parent.empty(); parent = parent.parent_path())
                if (archive->Entries.count(parent.generic_wstring())) throw std::runtime_error("Resource file/directory collision.");
        }
        input.clear();
        std::lock_guard<std::mutex> guard(MountMutex);
        Mounted = std::move(archive);
    }

    bool HasResourcePack() { return static_cast<bool>(Snapshot()); }

    void EnsureResourceWritable(const std::filesystem::path& path)
    {
        if (Snapshot() && !Writable(LogicalPath(path)))
            throw std::runtime_error("Packaged resources are read-only; write to UserData, Cache or Logs.");
    }

    std::vector<char> ReadResource(const std::filesystem::path& path)
    {
        const auto archive = Snapshot();
        if (archive)
        {
            const auto relative = LogicalPath(path);
            if (!Writable(relative))
            {
                const auto found = archive->Entries.find(relative.generic_wstring());
                if (found == archive->Entries.end()) throw std::runtime_error("Resource missing from mounted pack: " + relative.u8string());
                const auto& entry = found->second;
                if (entry.Size > std::vector<char>().max_size()) throw std::runtime_error("Resource exceeds addressable memory.");
                std::vector<char> bytes(static_cast<std::size_t>(entry.Size));
                std::lock_guard<std::mutex> guard(archive->Mutex);
                archive->Input.clear();
                archive->Input.seekg(static_cast<std::streamoff>(entry.Offset));
                Read(archive->Input, bytes.data(), bytes.size());
                return bytes;
            }
        }
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input) throw std::runtime_error("Resource file is unreadable: " + path.u8string());
        const auto length = input.tellg();
        if (length < 0 || static_cast<std::uint64_t>(length) > std::vector<char>().max_size())
            throw std::runtime_error("Resource file size is invalid.");
        std::vector<char> bytes(static_cast<std::size_t>(length));
        input.seekg(0); Read(input, bytes.data(), bytes.size());
        return bytes;
    }

    bool ResourceExists(const std::filesystem::path& path)
    {
        const auto archive = Snapshot();
        if (!archive) return std::filesystem::exists(path);
        const auto relative = LogicalPath(path);
        if (Writable(relative)) return std::filesystem::exists(path);
        return archive->Entries.count(relative.generic_wstring()) != 0 || ResourceIsDirectory(path);
    }

    bool ResourceIsDirectory(const std::filesystem::path& path)
    {
        const auto archive = Snapshot();
        if (!archive) return std::filesystem::is_directory(path);
        const auto relative = LogicalPath(path);
        if (Writable(relative)) return std::filesystem::is_directory(path);
        const auto prefix = relative.generic_wstring() + L'/';
        const auto found = archive->Entries.lower_bound(prefix);
        return found != archive->Entries.end() && found->first.size() >= prefix.size() &&
            CompareStringOrdinal(found->first.data(), static_cast<int>(prefix.size()), prefix.data(),
                static_cast<int>(prefix.size()), TRUE) == CSTR_EQUAL;
    }

    std::vector<std::filesystem::path> ListResourceDirectory(const std::filesystem::path& path)
    {
        const auto archive = Snapshot();
        std::vector<std::filesystem::path> result;
        if (!archive || Writable(LogicalPath(path)))
        {
            for (const auto& entry : std::filesystem::directory_iterator(path)) result.push_back(entry.path());
            return result;
        }
        const auto relative = LogicalPath(path);
        const auto prefix = relative.generic_wstring() + L'/';
        std::set<std::wstring, WindowsLess> children;
        for (auto entry = archive->Entries.lower_bound(prefix); entry != archive->Entries.end(); ++entry)
        {
            if (entry->first.size() < prefix.size() || CompareStringOrdinal(entry->first.data(),
                static_cast<int>(prefix.size()), prefix.data(), static_cast<int>(prefix.size()), TRUE) != CSTR_EQUAL) break;
            children.insert(entry->first.substr(prefix.size(), entry->first.find(L'/', prefix.size()) - prefix.size()));
        }
        for (const auto& child : children) result.push_back(relative / child);
        return result;
    }

    std::filesystem::path PreparePackagedResources(bool required)
    {
        std::wstring executable(32768, L'\0');
        const auto length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
        if (!length || length >= executable.size()) throw std::runtime_error("Cannot locate game executable.");
        executable.resize(length);
        auto pack = std::filesystem::path(executable); pack.replace_extension(L".gpak");
        if (!std::filesystem::exists(pack))
        {
            const auto categorized = pack.parent_path().parent_path() / L"Data" / pack.filename();
            if (std::filesystem::exists(categorized)) pack = categorized;
        }
        if (!std::filesystem::exists(pack))
        { if (required) throw std::runtime_error("Required resource pack is missing."); return {}; }
        MountResourcePack(pack);
        const auto archive = Snapshot();
        const auto root = pack.parent_path().filename() == L"Data" ? pack.parent_path().parent_path() : pack.parent_path();
        const auto config = root / L"Config" / L"Config.ini";
        if (!std::filesystem::is_regular_file(config) || std::filesystem::file_size(config) > 1024 * 1024)
            throw std::runtime_error("External Config/Config.ini is missing or exceeds 1 MiB.");
        const auto working = std::filesystem::absolute(config.parent_path().parent_path());
        archive->StateLock = CreateFileW((config.parent_path() / L".runtime.lock").c_str(), GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
        if (archive->StateLock == INVALID_HANDLE_VALUE) { archive->StateLock = nullptr; throw std::runtime_error("Game root is already in use or not writable."); }
        std::filesystem::current_path(working);
        return working;
    }
}
