#include <GEngine/Runtime/ResourcePack.h>
#include <windows.h>
#include <bcrypt.h>
#include <array>
#include <fstream>
#include <future>
#include <iostream>
#include <stdexcept>

namespace
{
    void Require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
    void Number(std::vector<char>& bytes, std::uint64_t value, int size)
    { for (int i = 0; i < size; ++i) bytes.push_back(static_cast<char>(value >> (8 * i))); }
    std::vector<char> Pack(const std::vector<std::pair<std::string, std::string>>& entries)
    {
        std::vector<char> bytes{'G','E','P','A','K','0','0','1'};
        Number(bytes, entries.size(), 4);
        for (const auto& entry : entries)
        {
            std::array<unsigned char, 32> hash{};
            Require(BCryptHash(BCRYPT_SHA256_ALG_HANDLE, nullptr, 0,
                reinterpret_cast<PUCHAR>(const_cast<char*>(entry.second.data())),
                static_cast<ULONG>(entry.second.size()), hash.data(), static_cast<ULONG>(hash.size())) >= 0,
                "Cannot hash test fixture");
            Number(bytes, entry.first.size(), 4); Number(bytes, entry.second.size(), 8);
            bytes.insert(bytes.end(), hash.begin(), hash.end());
            bytes.insert(bytes.end(), entry.first.begin(), entry.first.end());
            bytes.insert(bytes.end(), entry.second.begin(), entry.second.end());
        }
        return bytes;
    }
    void Write(const std::filesystem::path& path, const std::vector<char>& bytes)
    {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        Require(static_cast<bool>(output), "Cannot write fixture");
    }
    template<class F> void Reject(F&& work)
    { bool rejected = false; try { work(); } catch (const std::exception&) { rejected = true; } Require(rejected, "Invalid input was accepted"); }
}

int main(int argc, char** argv)
{
    try
    {
        Require(argc == 2, "Expected fixture directory");
        const auto root = std::filesystem::absolute(argv[1]);
        std::filesystem::create_directories(root);
        const std::string payload(131073, 'q');
        Write(root / "directory.txt", std::vector<char>(payload.begin(), payload.end()));
        const auto directory = GEngine::ReadResource(root / "directory.txt");
        const auto valid = Pack({{"Assets/Nested/data.txt", payload}, {"Assets/empty.txt", ""}});
        Write(root / "valid.gpak", valid);
        GEngine::MountResourcePack(root / "valid.gpak");
        Require(GEngine::HasResourcePack(), "Mount absent");
        const auto owned = GEngine::ReadResource("Assets/Nested/data.txt");
        Require(owned == directory, "Directory/archive bytes differ");
        Require(GEngine::ResourceExists("assets/nested/DATA.txt") &&
            GEngine::ResourceIsDirectory("Assets/Nested"), "Case-insensitive lookup failed");
        Require(GEngine::ResourceIsDirectory("Assets/Nested/") &&
            GEngine::ListResourceDirectory("Assets/Nested/").size() == 1, "Directory suffix/enumeration failed");
        Require(GEngine::ReadResource("Assets/empty.txt").empty(), "Empty entry failed");
        Reject([] { GEngine::ReadResource("Assets/missing.txt"); });
        Reject([] { GEngine::ReadResource("../escape"); });
        Reject([] { GEngine::EnsureResourceWritable("Assets/new.txt"); });
        Reject([] { GEngine::EnsureResourceWritable("../outside.txt"); });
        GEngine::EnsureResourceWritable("UserData/save.json");
        GEngine::EnsureResourceWritable("Cache/Shaders/");
        GEngine::EnsureResourceWritable("Config.ini");
        std::vector<std::future<void>> reads;
        for (int i = 0; i < 8; ++i)
            reads.push_back(std::async(std::launch::async, [&]
            {
                for (int j = 0; j < 25; ++j)
                    Require(GEngine::ReadResource("Assets/Nested/data.txt") == directory, "Concurrent read corruption");
            }));
        for (auto& read : reads) read.get();
        HANDLE writer = CreateFileW((root / "valid.gpak").c_str(), GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if (writer != INVALID_HANDLE_VALUE) CloseHandle(writer);
        Require(writer == INVALID_HANDLE_VALUE, "Mounted archive permits mutation");
        std::vector<std::vector<char>> invalid;
        auto bytes = valid; bytes[0] = '!'; invalid.push_back(bytes);
        bytes = valid; bytes.resize(10); invalid.push_back(bytes);
        bytes = valid; bytes.pop_back(); invalid.push_back(bytes);
        bytes = valid; bytes.push_back('x'); invalid.push_back(bytes);
        bytes = valid; bytes[24] ^= 1; invalid.push_back(bytes);
        bytes = valid; for (int i = 16; i < 24; ++i) bytes[i] = static_cast<char>(0xff); invalid.push_back(bytes);
        invalid.push_back(Pack({{"Assets/A", "a"}, {"assets/a", "b"}}));
        invalid.push_back(Pack({{"Assets/a", "a"}, {"Assets/a/b", "b"}}));
        for (const auto& path : {"../escape", "/root", "C:/bad", "Assets/../bad", "Assets//bad",
            "Assets/CON.txt", "Assets/trailing.", "Assets/a:stream", "Assets\\bad"})
            invalid.push_back(Pack({{path, "bad"}}));
        for (size_t i = 0; i < invalid.size(); ++i)
        {
            const auto path = root / ("bad-" + std::to_string(i) + ".gpak");
            Write(path, invalid[i]); Reject([&] { GEngine::MountResourcePack(path); });
            Require(GEngine::ReadResource("Assets/Nested/data.txt") == owned, "Failed mount replaced good archive");
        }
        Write(root / "replacement.gpak", Pack({{"Assets/new.txt", "replacement"}}));
        GEngine::MountResourcePack(root / "replacement.gpak");
        Require(owned == directory, "Owned bytes invalidated by remount");
        Require(GEngine::ReadResource("Assets/new.txt").size() == 11, "Replacement mount failed");
        DWORD handlesBefore = 0, handlesAfter = 0;
        Require(GetProcessHandleCount(GetCurrentProcess(), &handlesBefore) != 0, "Cannot sample handle count");
        for (int i = 0; i < 100; ++i)
            GEngine::MountResourcePack(root / (i % 2 ? "replacement.gpak" : "valid.gpak"));
        Require(GetProcessHandleCount(GetCurrentProcess(), &handlesAfter) != 0, "Cannot sample final handle count");
        Require(handlesAfter <= handlesBefore + 2, "Repeated mounts leak file/hash handles");
        Write(root / "empty.gpak", Pack({}));
        GEngine::MountResourcePack(root / "empty.gpak");
        Require(!GEngine::ResourceExists("Assets/missing"), "Empty archive lookup failed");
        std::cout << "PASS: directory/archive equality, 200 concurrent reads, owned lifetime, immutable file, "
            << invalid.size() << " malformed archives, failed-mount preservation and 100 remounts; handles "
            << handlesBefore << " -> " << handlesAfter << '\n';
        return 0;
    }
    catch (const std::exception& error) { std::cerr << "FAIL: " << error.what() << '\n'; return 1; }
}
