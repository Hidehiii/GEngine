#include "Protocol/EditorProtocol.h"
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <io.h>
#include <windows.h>

using namespace GEngineEditor::Protocol;

static bool Send(const Packet& packet)
{
    auto bytes = Encode(packet);
    std::cout.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    std::cout.flush();
    return static_cast<bool>(std::cout);
}

int main()
{
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter([](EXCEPTION_POINTERS*) -> LONG { return EXCEPTION_EXECUTE_HANDLER; });
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1)
        return 1;
    std::cerr << "Protocol diagnostic worker; no engine or scene renderer loaded.\n";
    Decoder decoder;
    std::uint64_t session = 0;
    bool ready = false;
    std::uint64_t lastRequest = 0;
    std::vector<std::uint8_t> input(4096);
    for (;;)
    {
        const int count = _read(_fileno(stdin), input.data(), static_cast<unsigned>(input.size()));
        if (count <= 0) return decoder.HasPartial() ? 2 : 0;
        std::vector<Packet> packets;
        if (!decoder.Feed(input.data(), count, packets)) return 2;
        for (const auto& packet : packets)
        {
            if (packet.Type == Kind::Hello && !ready)
            {
                if (Text(packet.Payload) != BuildId || packet.Request == 0) return 3;
                session = packet.Session;
                lastRequest = packet.Request;
                ready = true;
                if (!Send({Kind::Ready, session, packet.Request, Text(BuildId)})) return 1;
                // CPU-only orientation image; this is deliberately not an engine render.
                std::vector<std::uint8_t> image;
                Append(image, 128, 4); Append(image, 80, 4); Append(image, 128 * 4, 4);
                for (int y = 0; y < 80; ++y)
                    for (int x = 0; x < 128; ++x)
                    {
                        image.push_back(x < 64 ? 220 : 40);
                        image.push_back(y < 40 ? 160 : 40);
                        image.push_back(x < 64 && y < 40 ? 30 : 210);
                        image.push_back(255);
                    }
                if (!Send({Kind::DiagnosticFrame, session, 0, std::move(image)})) return 1;
                continue;
            }
            if (!ready || packet.Session != session || !packet.Payload.empty() ||
                packet.Request <= lastRequest) return 3;
            lastRequest = packet.Request;
            switch (packet.Type)
            {
            case Kind::Ping:
                if (!Send({Kind::Pong, session, packet.Request, {}})) return 1;
                break;
            case Kind::Shutdown:
                Send({Kind::Ack, session, packet.Request, {}});
                return 0;
            case Kind::Crash:
                // Raise an unhandled native fault only in this owned diagnostic process.
                RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 0, nullptr);
                return 4;
            case Kind::Hang:
                for (;;) std::this_thread::sleep_for(std::chrono::seconds(60));
            case Kind::Malformed:
                std::cout << "invalid protocol header deliberately injected" << std::flush;
                break;
            default: return 3;
            }
        }
    }
}
