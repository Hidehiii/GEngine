#include <GEngine/Asset/GAsset.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace GEngine
{
    GENGINE_API std::vector<char> ReadResource(const std::filesystem::path& path)
    {
        std::ifstream input(path, std::ios::binary);
        if (!input) throw std::runtime_error("Cannot read test fixture: " + path.u8string());
        return std::vector<char>(std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>());
    }
}

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    void Write(const std::filesystem::path& path, const std::string& contents)
    {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output << contents;
        Require(static_cast<bool>(output), "Cannot write test fixture");
    }

    template<class Function>
    void Reject(Function&& function, const char* message)
    {
        bool rejected = false;
        try { function(); }
        catch (const std::exception&) { rejected = true; }
        Require(rejected, message);
    }
}

int main(int argc, char** argv)
{
    try
    {
        Require(argc == 2 || argc == 3 || argc == 4,
            "Expected fixture directory and optional texture/mesh paths");
        const auto root = std::filesystem::absolute(argv[1]);
        std::filesystem::create_directories(root);

        const auto textPath = root / "text.gasset";
        Write(textPath,
            "schemaVersion: 1\n"
            "type: shader\n"
            "format: shader\n"
            "payloadEncoding: text\n"
            "payload: |\n"
            "    Shader \"Fixture\"\n");
        const auto text = GEngine::LoadGAssetText(textPath, "shader");
        Require(text == "Shader \"Fixture\"\n", "Text payload changed");

        const auto binaryPath = root / "binary.gasset";
        Write(binaryPath,
            "schemaVersion: 1\n"
            "type: texture\n"
            "format: png\n"
            "payloadEncoding: base64\n"
            "payload: !!binary AAEC/w==\n");
        const auto bytes = GEngine::LoadGAssetBinary(binaryPath, "texture");
        const std::vector<char> expected{0, 1, 2, static_cast<char>(0xff)};
        Require(bytes == expected, "Binary payload changed");

        if (argc == 3)
        {
            const auto probe = GEngine::LoadGAssetBinary(argv[2], "texture");
            Require(!probe.empty(), "Production texture payload is empty");
        }
        if (argc == 4)
        {
            const auto mesh = GEngine::LoadGAssetBinary(argv[3], "mesh");
            Require(mesh.size() > 1024, "Production mesh payload is empty");
        }

        Reject([&] { GEngine::LoadGAsset(textPath, "texture"); }, "Wrong expected type accepted");
        Reject([&] { GEngine::LoadGAsset(root / "raw.shader", "shader"); }, "Wrong extension accepted");
        Reject([&]
        {
            Write(root / "version.gasset",
                "schemaVersion: 2\n"
                "type: shader\nformat: shader\npayloadEncoding: text\npayload: x\n");
            GEngine::LoadGAsset(root / "version.gasset", "shader");
        }, "Unsupported schema accepted");
        Reject([&]
        {
            Write(root / "unknown.gasset",
                "schemaVersion: 1\nunknown: yes\n"
                "type: shader\nformat: shader\npayloadEncoding: text\npayload: x\n");
            GEngine::LoadGAsset(root / "unknown.gasset", "shader");
        }, "Unknown field accepted");
        Reject([&]
        {
            Write(root / "encoding.gasset",
                "schemaVersion: 1\ntype: shader\nformat: shader\n"
                "payloadEncoding: hex\npayload: 00\n");
            GEngine::LoadGAsset(root / "encoding.gasset", "shader");
        }, "Unknown payload encoding accepted");

        std::cout << "PASS: GAsset schema, text/base64 payload and rejection checks\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
