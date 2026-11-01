#include "GEngine/Graphics/ShaderContainer.h"
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace GEngine
{
    namespace
    {
        std::string Lower(std::string value)
        {
            for (char& c : value) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
            return value;
        }

        template<class T> T EnumValue(const std::string& value,
            std::initializer_list<std::pair<const char*, T>> values)
        {
            const auto key = Lower(value);
            for (const auto& entry : values) if (key == entry.first) return entry.second;
            throw std::invalid_argument("Unknown value '" + value + "'");
        }
    }

    // Compatibility conversion helpers retain their existing signatures.
    namespace Utils
    {
        CompareOperation ShaderCompareOperationFromString(const std::string& value)
        {
            return EnumValue<CompareOperation>(value, {{"less", COMPARE_OP_LESS}, {"greater", COMPARE_OP_GREATER},
                {"lessequal", COMPARE_OP_LESS_EQUAL}, {"lequal", COMPARE_OP_LESS_EQUAL},
                {"greaterequal", COMPARE_OP_GREATER_EQUAL}, {"gequal", COMPARE_OP_GREATER_EQUAL},
                {"equal", COMPARE_OP_EQUAL}, {"notequal", COMPARE_OP_NOT_EQUAL}, {"always", COMPARE_OP_ALWAYS}});
        }
        CullMode ShaderCullModeFromString(const std::string& value)
        {
            return EnumValue<CullMode>(value, {{"none", CULL_MODE_NONE}, {"off", CULL_MODE_NONE},
                {"back", CULL_MODE_BACK}, {"front", CULL_MODE_FRONT}});
        }
        BlendFactor ShaderBlendFactorFromString(const std::string& value)
        {
            return EnumValue<BlendFactor>(value, {{"srcalpha", BLEND_FACTOR_SRC_ALPHA}, {"dstalpha", BLEND_FACTOR_DST_ALPHA},
                {"srccolor", BLEND_FACTOR_SRC_COLOR}, {"dstcolor", BLEND_FACTOR_DST_COLOR},
                {"oneminussrcalpha", BLEND_FACTOR_ONE_MINUS_SRC_ALPHA}, {"oneminusdstalpha", BLEND_FACTOR_ONE_MINUS_DST_ALPHA},
                {"oneminussrccolor", BLEND_FACTOR_ONE_MINUS_SRC_COLOR}, {"oneminusdstcolor", BLEND_FACTOR_ONE_MINUS_DST_COLOR},
                {"one", BLEND_FACTOR_ONE}, {"zero", BLEND_FACTOR_ZERO}});
        }
        BlendMode ShaderBlendModeFromString(const std::string& value)
        {
            return EnumValue<BlendMode>(value, {{"none", BLEND_MODE_NONE}, {"add", BLEND_MODE_ADD},
                {"substract", BLEND_MODE_SUBSTRACT}, {"subtract", BLEND_MODE_SUBSTRACT},
                {"reversesubstract", BLEND_MODE_REVERSE_SUBSTRACT}, {"reversesubtract", BLEND_MODE_REVERSE_SUBSTRACT},
                {"min", BLEND_MODE_MIN}, {"max", BLEND_MODE_MAX}});
        }
    }

    namespace
    {
        bool IdentifierStart(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
        bool IdentifierPart(char c) { return IdentifierStart(c) || (c >= '0' && c <= '9'); }

        struct Token
        {
            std::string Text;
            size_t Offset = 0, Line = 1;
            bool Quoted = false;
        };

        class Parser
        {
        public:
            Parser(std::string_view source, std::string_view name) : m_Source(source), m_Name(name)
            {
                if (m_Source.substr(0, 3) == "\xEF\xBB\xBF") m_Pos = 3;
                const auto nul = source.find('\0');
                if (nul != std::string_view::npos) Fail(nul, "NUL byte in shader container");
            }

            ShaderContainer Parse()
            {
                Expect("shader");
                const auto name = Take();
                if (!name.Quoted || name.Text.empty()) Fail(name.Offset, "Expected nonempty quoted shader name");
                ShaderContainer result;
                result.Name = name.Text;
                Expect("{");
                while (Is(Peek(), "pass")) result.Passes.push_back(Pass());
                Expect("}");
                if (!End(Peek())) Fail(Peek().Offset, "Unexpected input after Shader block");
                if (result.Passes.empty()) Fail(name.Offset, "Shader requires at least one Pass");
                return result;
            }

        private:
            [[noreturn]] void Fail(size_t offset, const std::string& message) const
            {
                size_t line = 1, column = 1;
                for (size_t i = 0; i < offset; ++i)
                    if (m_Source[i] == '\n') { ++line; column = 1; } else ++column;
                throw std::invalid_argument("Shader parse error " + std::string(m_Name) + ":" +
                    std::to_string(line) + ":" + std::to_string(column) + ": " + message);
            }

            char Current() const { return m_Pos < m_Source.size() ? m_Source[m_Pos] : '\0'; }
            bool Starts(std::string_view text) const { return m_Source.substr(m_Pos, text.size()) == text; }
            void Advance() { if (Current() == '\n') ++m_Line; ++m_Pos; }
            void Comment()
            {
                const size_t begin = m_Pos;
                const bool block = Starts("/*");
                Advance(); Advance();
                if (block)
                {
                    while (Current() && !Starts("*/")) Advance();
                    if (!Current()) Fail(begin, "Unterminated block comment");
                    Advance(); Advance();
                }
                else while (Current() && Current() != '\n') Advance();
            }
            void Trivia()
            {
                for (;;)
                {
                    if (Current() && (Current() == ' ' || Current() == '\t' || Current() == '\r' || Current() == '\n')) Advance();
                    else if (Starts("//") || Starts("/*")) Comment();
                    else return;
                }
            }
            Token Read()
            {
                Trivia();
                Token token{ "", m_Pos, m_Line, false };
                if (!Current()) return token;
                if (Current() == '"')
                {
                    token.Quoted = true;
                    Advance();
                    while (Current() && Current() != '"')
                    {
                        if (Current() == '\r' || Current() == '\n') Fail(token.Offset, "Newline in quoted value");
                        if (Current() == '\\')
                        {
                            Advance();
                            if (Current() != '\\' && Current() != '"') Fail(m_Pos, "Expected escaped quote or backslash");
                        }
                        token.Text += Current(); Advance();
                    }
                    if (!Current()) Fail(token.Offset, "Unterminated quoted value");
                    Advance();
                }
                else if (IdentifierPart(Current()))
                    do { token.Text += Current(); Advance(); } while (IdentifierPart(Current()));
                else { token.Text += Current(); Advance(); }
                return token;
            }
            const Token& Peek() { if (!m_HasToken) { m_Token = Read(); m_HasToken = true; } return m_Token; }
            Token Take() { Token result = Peek(); m_HasToken = false; return result; }
            bool Is(const Token& token, const char* word) const { return !token.Quoted && Lower(token.Text) == word; }
            bool End(const Token& token) const { return token.Offset == m_Source.size(); }
            void Expect(const char* word)
            {
                const auto token = Take();
                if (!Is(token, word)) Fail(token.Offset, "Expected '" + std::string(word) + "'");
            }

            std::string Program()
            {
                Expect("{");
                const size_t begin = m_Pos;
                size_t depth = 1;
                while (Current())
                {
                    if (Starts("//") || Starts("/*")) { Comment(); continue; }
                    if (Current() == '"' || Current() == '\'')
                    {
                        const size_t quoteOffset = m_Pos;
                        const char quote = Current(); Advance();
                        while (Current() && Current() != quote)
                        {
                            if (Current() == '\r' || Current() == '\n') Fail(quoteOffset, "Unterminated Program string");
                            if (Current() == '\\')
                            {
                                Advance();
                                if (!Current()) Fail(quoteOffset, "Unterminated Program escape");
                            }
                            Advance();
                        }
                        if (!Current()) Fail(quoteOffset, "Unterminated Program string");
                        Advance(); continue;
                    }
                    if (Current() == '{') ++depth;
                    if (Current() == '}' && --depth == 0)
                    {
                        const auto code = std::string(m_Source.substr(begin, m_Pos - begin));
                        Advance(); return code;
                    }
                    Advance();
                }
                Fail(begin, "Unterminated Program block");
            }

            void State(ShaderContainerPass& pass, const Token& directive, const std::vector<Token>& args)
            {
                const auto key = Lower(directive.Text);
                auto count = [&](size_t a, size_t b = 0)
                {
                    if (args.size() != a && (!b || args.size() != b)) Fail(directive.Offset, "Invalid argument count for " + directive.Text);
                };
                auto value = [&](size_t i) -> const std::string&
                {
                    if (args[i].Quoted) Fail(args[i].Offset, "Expected unquoted state value");
                    return args[i].Text;
                };
                auto& state = pass.State;
                try
                {
                    if (key == "cull") { count(1); state.Cull = Utils::ShaderCullModeFromString(value(0)); }
                    else if (key == "depthtest") { count(1); state.DepthTestOp = Utils::ShaderCompareOperationFromString(value(0)); }
                    else if (key == "depthwrite")
                    {
                        count(1);
                        state.DepthWrite = EnumValue<bool>(value(0), {{"on", true}, {"true", true}, {"1", true},
                            {"off", false}, {"false", false}, {"0", false}});
                    }
                    else if (key == "blend")
                    {
                        count(2, 4);
                        state.BlendColorSrc = Utils::ShaderBlendFactorFromString(value(0));
                        state.BlendColorDst = Utils::ShaderBlendFactorFromString(value(1));
                        state.BlendAlphaSrc = Utils::ShaderBlendFactorFromString(value(args.size() == 4 ? 2 : 0));
                        state.BlendAlphaDst = Utils::ShaderBlendFactorFromString(value(args.size() == 4 ? 3 : 1));
                    }
                    else if (key == "blendop")
                    {
                        count(1, 2);
                        state.BlendColor = Utils::ShaderBlendModeFromString(value(0));
                        state.BlendAlpha = Utils::ShaderBlendModeFromString(value(args.size() == 2 ? 1 : 0));
                    }
                    else if (key == "colormask")
                    {
                        if (args.empty()) Fail(directive.Offset, "ColorMask requires channels or 0");
                        state.ColorMask = 0;
                        for (const auto& arg : args)
                        {
                            if (arg.Quoted) Fail(arg.Offset, "Expected unquoted ColorMask channel");
                            if (arg.Text == "0" && args.size() == 1) continue;
                            const auto bit = EnumValue<uint32_t>(arg.Text, {{"r", COLOR_MASK_CHANNEL_R}, {"g", COLOR_MASK_CHANNEL_G},
                                {"b", COLOR_MASK_CHANNEL_B}, {"a", COLOR_MASK_CHANNEL_A}});
                            if (state.ColorMask & bit) Fail(arg.Offset, "Duplicate ColorMask channel");
                            state.ColorMask |= bit;
                        }
                    }
                    else if (key == "tag") { count(1); if (args[0].Text.empty()) Fail(args[0].Offset, "Empty Tag"); state.Tag = args[0].Text; }
                    else Fail(directive.Offset, "Unknown pass directive '" + directive.Text + "'");
                }
                catch (const std::invalid_argument& error)
                {
                    // Conversion errors gain the source location of their directive.
                    if (std::string(error.what()).find("Shader parse error ") == 0) throw;
                    Fail(directive.Offset, directive.Text + ": " + error.what());
                }
            }

            ShaderContainerPass Pass()
            {
                const auto start = Take();
                Expect("{");
                ShaderContainerPass pass;
                std::unordered_set<std::string> seen;
                while (!Is(Peek(), "program"))
                {
                    auto directive = Take();
                    if (End(directive) || Is(directive, "}")) Fail(directive.Offset, "Pass requires a Program block");
                    std::string key = Lower(directive.Text);
                    const bool pragma = key == "#" && !directive.Quoted;
                    if (pragma)
                    {
                        const auto word = Take();
                        if (!Is(word, "pragma") || word.Line != directive.Line) Fail(word.Offset, "Expected #pragma stage entry");
                    }
                    else if (directive.Quoted || !IdentifierStart(directive.Text[0])) Fail(directive.Offset, "Expected pass directive");
                    std::vector<Token> args;
                    while (!End(Peek()) && Peek().Line == directive.Line && !Is(Peek(), "}") && !Is(Peek(), "{"))
                    {
                        auto arg = Take();
                        if (Is(arg, ",") && key == "colormask") continue;
                        args.push_back(std::move(arg));
                    }
                    if (pragma)
                    {
                        if (args.size() != 2) Fail(directive.Offset, "Expected #pragma stage entry");
                        const auto stage = Lower(args[0].Text);
                        const auto& entry = args[1].Text;
                        const std::unordered_set<std::string> stages = {"vertex", "fragment", "hull", "domain", "geometry", "compute", "amplification", "mesh"};
                        if (args[0].Quoted || !stages.count(stage)) Fail(args[0].Offset, "Unknown shader stage '" + stage + "'");
                        if (args[1].Quoted || entry.empty() || !IdentifierStart(entry[0]) ||
                            !std::all_of(entry.begin(), entry.end(), IdentifierPart)) Fail(args[1].Offset, "Invalid entry-point identifier");
                        if (!pass.EntryPoints.emplace(stage, entry).second) Fail(args[0].Offset, "Duplicate shader stage '" + stage + "'");
                    }
                    else
                    {
                        if (!seen.insert(key).second) Fail(directive.Offset, "Duplicate pass directive '" + directive.Text + "'");
                        State(pass, directive, args);
                    }
                }
                Take();
                pass.Program = Program();
                Expect("}");
                const auto has = [&](const char* stage) { return pass.EntryPoints.count(stage) != 0; };
                if (pass.EntryPoints.empty()) Fail(start.Offset, "Pass requires stage entry points");
                if (has("compute") && pass.EntryPoints.size() != 1) Fail(start.Offset, "Compute cannot mix with graphics stages");
                if (has("amplification") && !has("mesh")) Fail(start.Offset, "Amplification requires mesh");
                if (has("mesh") && (has("vertex") || has("hull") || has("domain") || has("geometry"))) Fail(start.Offset, "Mesh cannot mix with legacy vertex stages");
                if (has("hull") != has("domain")) Fail(start.Offset, "Hull and domain must be paired");
                if (!has("compute") && !has("mesh") && !has("vertex")) Fail(start.Offset, "Graphics pass requires vertex or mesh");
                pass.MeshWorkflow = has("mesh");
                return pass;
            }

            std::string_view m_Source, m_Name;
            size_t m_Pos = 0, m_Line = 1;
            Token m_Token;
            bool m_HasToken = false;
        };
    }

    ShaderContainer ParseShaderContainer(std::string_view source, std::string_view sourceName)
    {
        return Parser(source, sourceName).Parse();
    }
}
