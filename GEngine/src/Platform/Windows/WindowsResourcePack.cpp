#include "GEpch.h"
#include "GEngine/Runtime/ResourcePack.h"
#include <Windows.h>
#include <bcrypt.h>
#include <array>
#include <fstream>
#include <set>
#include <stdexcept>
#include <algorithm>
#include <cwctype>

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
    }
    std::filesystem::path PreparePackagedResources(bool required)
    {
        std::wstring executable(32768, L'\0');
        const auto length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
        if (!length || length >= executable.size()) throw std::runtime_error("Cannot locate game executable.");
        executable.resize(length);
        auto pack = std::filesystem::path(executable); pack.replace_extension(L".gpak");
        if (!std::filesystem::exists(pack))
        { if (required) throw std::runtime_error("Required resource pack is missing."); return {}; }
        std::ifstream input(pack, std::ios::binary);
        std::array<char, 8> magic{}; Read(input, magic.data(), magic.size());
        if (std::string(magic.data(), magic.size()) != "GEPAK001") throw std::runtime_error("Unsupported resource pack version.");
        const auto count = Number(input, 4);
        if (!count || count > 100000) throw std::runtime_error("Invalid resource count.");
        std::wstring temporary(32768, L'\0');
        const auto tempLength = GetTempPathW(static_cast<DWORD>(temporary.size()), temporary.data());
        if (!tempLength || tempLength >= temporary.size()) throw std::runtime_error("Cannot locate writable game state directory.");
        temporary.resize(tempLength);
        const auto parent = std::filesystem::path(temporary) / L"GEngineGames";
        std::filesystem::create_directories(parent);
        if (GetFileAttributesW(parent.c_str()) & FILE_ATTRIBUTE_REPARSE_POINT) throw std::runtime_error("Linked game state directory rejected.");
        std::array<unsigned char, 16> random{};
        if (BCryptGenRandom(nullptr, random.data(), static_cast<ULONG>(random.size()), BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0) throw std::runtime_error("Cannot allocate game session identity.");
        std::wstring name;
        for (auto c : random) { name += L"0123456789abcdef"[c >> 4]; name += L"0123456789abcdef"[c & 15]; }
        const auto working = parent / name;
        if (!CreateDirectoryW(working.c_str(), nullptr)) throw std::runtime_error("Cannot create private game session.");
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
            if (!paths.insert(relative.wstring()).second) throw std::runtime_error("Duplicate resource path.");
            const auto position = static_cast<std::uint64_t>(input.tellg());
            if (position > packSize || size > packSize - position) throw std::runtime_error("Truncated resource payload.");
            const auto target = working / relative;
            std::filesystem::create_directories(target.parent_path());
            std::ofstream output(target, std::ios::binary);
            if (!output) throw std::runtime_error("Cannot extract resource.");
            Hash hash;
            std::array<unsigned char, 65536> buffer{};
            for (std::uint64_t remaining = size; remaining;)
            {
                const auto bytes = static_cast<std::size_t>((std::min)(remaining, static_cast<std::uint64_t>(buffer.size())));
                Read(input, buffer.data(), bytes); hash.Append(buffer.data(), static_cast<ULONG>(bytes));
                output.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(bytes));
                if (!output) throw std::runtime_error("Resource extraction write failed.");
                remaining -= bytes;
            }
            output.close();
            if (!output || hash.Finish() != expected) throw std::runtime_error("Resource checksum mismatch.");
        }
        if (input.peek() != std::char_traits<char>::eof()) throw std::runtime_error("Unexpected trailing resource data.");
        std::filesystem::current_path(working);
        return working;
    }
}
