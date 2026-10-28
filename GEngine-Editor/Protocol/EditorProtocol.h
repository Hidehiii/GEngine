#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

namespace GEngineEditor::Protocol
{
    constexpr std::uint16_t Version = 1;
    constexpr std::size_t HeaderSize = 32;
    constexpr std::size_t MaxPayload = 64 * 1024;
    constexpr const char* BuildId = "GEngine.EditorDiagnostic/1";
    constexpr const char* EngineBuildId = "GEngine.EditorEngine/2";
    constexpr unsigned MaxFrameWidth = 1280, MaxFrameHeight = 720;
    enum class Kind : std::uint16_t
    {
        Hello = 1, Ready, Ping, Pong, Shutdown, Ack, DiagnosticFrame,
        Crash, Hang, Malformed, EngineFrame, RequestFrame, FrameBegin, FrameChunk, FrameEnd
    };
    struct Packet
    {
        Kind Type = Kind::Hello;
        std::uint64_t Session = 0;
        std::uint64_t Request = 0;
        std::vector<std::uint8_t> Payload;
    };
    inline void Append(std::vector<std::uint8_t>& data, std::uint64_t value, unsigned bytes)
    {
        for (unsigned i = 0; i < bytes; ++i) data.push_back(static_cast<std::uint8_t>(value >> (i * 8)));
    }
    inline std::uint64_t Read(const std::uint8_t* data, unsigned bytes)
    {
        std::uint64_t value = 0;
        for (unsigned i = 0; i < bytes; ++i) value |= static_cast<std::uint64_t>(data[i]) << (i * 8);
        return value;
    }
    inline std::vector<std::uint8_t> Text(const std::string& value) { return {value.begin(), value.end()}; }
    inline std::string Text(const std::vector<std::uint8_t>& value) { return {value.begin(), value.end()}; }
    inline std::vector<std::uint8_t> Encode(const Packet& packet)
    {
        const auto type = static_cast<std::uint16_t>(packet.Type);
        if (packet.Session == 0 || type < 1 || type > 15 || packet.Payload.size() > MaxPayload)
            throw std::invalid_argument("Invalid editor protocol packet");
        std::vector<std::uint8_t> result{'G','E','I','P'};
        Append(result, Version, 2);
        Append(result, type, 2);
        Append(result, packet.Session, 8);
        Append(result, packet.Request, 8);
        Append(result, packet.Payload.size(), 4);
        Append(result, 0, 4);
        result.insert(result.end(), packet.Payload.begin(), packet.Payload.end());
        return result;
    }
    class Decoder
    {
    public:
        bool Feed(const std::uint8_t* data, std::size_t size, std::vector<Packet>& packets)
        {
            if (!m_Error.empty()) return false;
            if (size > MaxPayload || m_Buffer.size() + size > 2 * (HeaderSize + MaxPayload))
                return Fail("Receive buffer limit exceeded");
            if (size) m_Buffer.insert(m_Buffer.end(), data, data + size);
            std::size_t consumed = 0;
            while (m_Buffer.size() - consumed >= HeaderSize)
            {
                const auto* header = m_Buffer.data() + consumed;
                const auto type = Read(header + 6, 2);
                const auto payload = Read(header + 24, 4);
                if (std::memcmp(header, "GEIP", 4) != 0 || Read(header + 4, 2) != Version ||
                    type < 1 || type > 15 || Read(header + 8, 8) == 0 || Read(header + 28, 4) != 0)
                    return Fail("Invalid packet header or protocol version");
                if (payload > MaxPayload) return Fail("Payload limit exceeded");
                if (m_Buffer.size() - consumed < HeaderSize + payload) break;
                Packet packet;
                packet.Type = static_cast<Kind>(type);
                packet.Session = Read(header + 8, 8);
                packet.Request = Read(header + 16, 8);
                packet.Payload.assign(header + HeaderSize, header + HeaderSize + payload);
                packets.push_back(std::move(packet));
                consumed += HeaderSize + static_cast<std::size_t>(payload);
            }
            m_Buffer.erase(m_Buffer.begin(), m_Buffer.begin() + consumed);
            return true;
        }
        bool HasPartial() const { return !m_Buffer.empty(); }
        const std::string& Error() const { return m_Error; }
        void Reset() { m_Buffer.clear(); m_Error.clear(); }
    private:
        bool Fail(const char* error) { m_Error = error; return false; }
        std::vector<std::uint8_t> m_Buffer;
        std::string m_Error;
    };
}
