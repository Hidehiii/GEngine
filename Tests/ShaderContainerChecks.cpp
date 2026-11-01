#include "GEngine/Graphics/ShaderContainer.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <stdexcept>

using namespace GEngine;
// Shared enum headers expose logger accessors; the CPU harness owns their storage.
namespace GEngine
{
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
}
namespace
{
    void Check(bool value, const char* message)
    {
        if (!value) throw std::runtime_error(message);
    }
    std::string Wrap(const std::string& state, const std::string& stages = "#pragma vertex vert\n#pragma fragment frag\n",
        const std::string& body = "float4 frag() : SV_Target { return 1; }")
    {
        return "Shader \"Example\" { Pass {\n" + state + "\n" + stages + "Program {" + body + "}\n}}";
    }
    size_t rejected = 0;
    void Reject(const std::string& source, const std::string& reason)
    {
        try { ParseShaderContainer(source, "regression.shader"); }
        catch (const std::invalid_argument& error)
        {
            const std::string message = error.what();
            Check(message.find("Shader parse error regression.shader:") == 0, "Missing source location");
            if (message.find(reason) == std::string::npos) throw std::runtime_error("Unexpected diagnostic: " + message + " (expected " + reason + ")");
            ++rejected; return;
        }
        throw std::runtime_error("Malformed source was accepted: " + reason);
    }
    void Regressions()
    {
        const std::string body = "\n// Pass Program { }\n/* } { */\n#define WORD \"} Pass {\"\nfloat4 frag() : SV_Target { return 1; }";
        const auto parsed = ParseShaderContainer("\xEF\xBB\xBF/* Shader \"fake\" { */\n" + Wrap(
            "Cull Off\nDepthTest GEQual\nDepthWrite On\nBlend SrcAlpha OneMinusSrcAlpha One Zero\nBlendOp Subtract ReverseSubstract\nColorMask R,G,B\nTag \"Forward Pass\"", "#pragma VERTEX vert\n#pragma fragment frag\n", body));
        const auto& pass = parsed.Passes.at(0);
        Check(parsed.Name == "Example" && pass.Program == body, "Program bytes or shader name changed");
        Check(pass.State.Cull == CULL_MODE_NONE && pass.State.DepthWrite && pass.State.DepthTestOp == COMPARE_OP_GREATER_EQUAL, "State conversion mismatch");
        Check(pass.State.ColorMask == 7 && pass.State.Tag == "Forward Pass", "Mask or quoted Tag mismatch");
        Check(pass.State.BlendColor == BLEND_MODE_SUBSTRACT && pass.State.BlendAlpha == BLEND_MODE_REVERSE_SUBSTRACT &&
            pass.State.BlendAlphaSrc == BLEND_FACTOR_ONE && pass.State.BlendColorSrc == BLEND_FACTOR_SRC_ALPHA, "Separate alpha blend mismatch");
        Check(pass.EntryPoints.at("vertex") == "vert" && !pass.MeshWorkflow, "Stage mismatch");
        for (const std::string value : {"On", "TRUE", "1"}) Check(ParseShaderContainer(Wrap("DepthWrite " + value)).Passes[0].State.DepthWrite, "DepthWrite enabled alias failed");
        for (const std::string value : {"Off", "FALSE", "0"}) Check(!ParseShaderContainer(Wrap("DepthWrite " + value)).Passes[0].State.DepthWrite, "DepthWrite disabled alias failed");
        const auto defaults = ParseShaderContainer(Wrap(""));
        Check(defaults.Passes[0].State == RenderState{}, "Default render state changed");
        Check(ParseShaderContainer(Wrap("ColorMask 0")).Passes[0].State.ColorMask == 0, "ColorMask 0 failed");
        Check(ParseShaderContainer(Wrap("", "#pragma compute main\n")).Passes[0].EntryPoints.count("compute") == 1, "Compute stage rejected");
        Check(ParseShaderContainer(Wrap("", "#pragma mesh main\n#pragma amplification amp\n")).Passes[0].MeshWorkflow, "Mesh workflow missing");
        Check(ParseShaderContainer(Wrap("", "#pragma vertex vert\n#pragma hull hull\n#pragma domain domain\n")).Passes[0].EntryPoints.size() == 3, "Tessellation pair rejected");
        const auto multi = ParseShaderContainer("Shader \"Multi\" { Pass {\n#pragma vertex v\nProgram {a}} Pass {\n#pragma compute c\nProgram {b}}}");
        Check(multi.Passes.size() == 2 && multi.Passes[0].Program == "a" && multi.Passes[1].Program == "b", "Multiple passes or final byte extraction failed");
        auto crlf = Wrap("Cull /* token boundary */ Front // trailing comment\nDepthWrite Off");
        for (size_t at = 0; (at = crlf.find('\n', at)) != std::string::npos; at += 2) crlf.replace(at, 1, "\r\n");
        Check(ParseShaderContainer(crlf).Passes[0].State.Cull == CULL_MODE_FRONT, "CRLF/comments failed");
        const std::string quotedProgram = R"hlsl(const char c = '}'; const char* s = "escaped \" } /* {"; // }
{})hlsl";
        Check(ParseShaderContainer(Wrap("", "#pragma vertex v\n", quotedProgram)).Passes[0].Program == quotedProgram, "Escaped strings/character literals changed program boundaries");
        Reject("", "Expected 'shader'");
        Reject("Shader Name {}", "quoted shader name");
        Reject("Shader \"\" {}", "quoted shader name");
        Reject("Shader \"unterminated", "Unterminated quoted");
        Reject("Shader \"x\n\" {}", "Newline");
        Reject("Shader \"X\" {}", "at least one Pass");
        Reject("Shader \"X\" {Pass {}}", "requires a Program");
        Reject(Wrap("") + " trailing", "Unexpected input");
        Reject(Wrap("") + "\"\"", "Unexpected input");
        Reject(Wrap("") + " /*", "Unterminated block comment");
        Reject(std::string("Shader\0", 7), "NUL");
        Reject(Wrap("Cull"), "argument count");
        Reject(Wrap("Cull Sideways"), "regression.shader:2:1: Cull: Unknown value");
        Reject(Wrap("DepthTest NeverTypo"), "Unknown value");
        Reject(Wrap("DepthWrite Sometimes"), "Unknown value");
        Reject(Wrap("DepthWrite \"\""), "unquoted");
        Reject(Wrap("Cull Back\nCull Front"), "Duplicate pass directive");
        Reject(Wrap("UnknownDirective foo"), "Unknown pass directive");
        Reject(Wrap("Blend One"), "argument count");
        Reject(Wrap("Blend Mystery Zero"), "Unknown value");
        Reject(Wrap("BlendOp Mystery"), "Unknown value");
        Reject(Wrap("ColorMask"), "requires channels");
        Reject(Wrap("ColorMask R Z"), "Unknown value");
        Reject(Wrap("ColorMask R R"), "Duplicate ColorMask");
        Reject(Wrap("Tag"), "argument count");
        Reject(Wrap("Tag \"\""), "Empty Tag");
        Reject(Wrap("", ""), "requires stage");
        Reject(Wrap("", "#pragma vertex\n"), "Expected #pragma");
        Reject(Wrap("", "#pragma unknown main\n"), "Unknown shader stage");
        Reject(Wrap("", "#pragma vertex 123\n"), "Invalid entry-point");
        Reject(Wrap("", "#pragma vertex v\n#pragma VERTEX v\n"), "Duplicate shader stage");
        Reject(Wrap("", "#pragma vertex v\n#pragma compute c\n"), "cannot mix");
        Reject(Wrap("", "#pragma amplification a\n"), "requires mesh");
        Reject(Wrap("", "#pragma mesh m\n#pragma vertex v\n"), "cannot mix");
        Reject(Wrap("", "#pragma vertex v\n#pragma hull h\n"), "must be paired");
        Reject(Wrap("", "#pragma fragment f\n"), "requires vertex or mesh");
        Reject(Wrap("", "#pragma vertex v\n", "/* unterminated"), "Unterminated block comment");
        Reject(Wrap("", "#pragma vertex v\n", "\"unterminated"), "Unterminated Program string");
        Reject(Wrap("", "#pragma vertex v\n", "{unclosed"), "Expected '}'");
        const auto complete = Wrap("");
        for (size_t size = 0; size < complete.size(); ++size) Reject(complete.substr(0, size), "");
        // Deterministic arbitrary inputs exercise scanner bounds and termination.
        std::mt19937 random(71237);
        const std::string alphabet = "ShaderPassProgram{}\"'/*#pragma\n\r\\ abc123";
        for (size_t i = 0; i < 2000; ++i)
        {
            std::string input;
            for (size_t size = random() % 256; size; --size) input += alphabet[random() % alphabet.size()];
            Reject(input, "");
        }
        std::cout << "PASS parser semantics, comments/strings, multi-pass, " << rejected << " malformed/truncated/generated inputs\n";
    }
}

int main(int argc, char** argv)
{
    try
    {
        Regressions();
        for (int i = 1; i < argc; ++i)
        {
            const bool reject = std::string(argv[i]) == "--reject-file";
            if (reject && ++i == argc) throw std::runtime_error("Missing rejected corpus path");
            std::ifstream input(argv[i], std::ios::binary);
            if (!input) throw std::runtime_error("Cannot open corpus file");
            const std::string source{std::istreambuf_iterator<char>(input), {}};
            if (reject) { Reject(source, ""); std::cout << "PASS rejected corpus " << argv[i] << '\n'; continue; }
            const auto parsed = ParseShaderContainer(source, argv[i]);
            std::cout << "PASS corpus " << argv[i] << " (" << parsed.Passes.size() << " passes)\n";
        }
        return 0;
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
