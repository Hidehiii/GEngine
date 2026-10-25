#include <GEngine.h>
#include <GEngine/Runtime/EngineRuntime.h>
#include <GEngine/Renderer/RenderGraph.h>
#include <GEngine/Graphics/RenderDevice.h>
#include <GEngine/Graphics/GraphicsPresent.h>
#include "Protocol/EditorProtocol.h"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cstdio>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>

namespace
{
    using namespace GEngineEditor::Protocol;
    FILE* output = nullptr;
    std::uint64_t session = 0, lastRequest = 0;
    Decoder decoder;
    void Send(Kind type, std::uint64_t request, std::vector<std::uint8_t> payload = {})
    {
        const auto bytes = Encode({type, session, request, std::move(payload)});
        if (fwrite(bytes.data(), 1, bytes.size(), output) != bytes.size() || fflush(output) != 0)
            throw std::runtime_error("Editor pipe disconnected.");
    }
    class Preview final : public GEngine::Layer
    {
    public:
        explicit Preview(GEngine::Application& runtime) : m_Runtime(runtime) {}
        void OnAttach() override
        {
            using namespace GEngine;
            auto& device = Graphics::GetRenderDevice();
            const float vertices[] = {0, 0.7f, 0, 0.7f, -0.55f, 0, -0.7f, -0.55f, 0};
            m_Pipeline = device.CreateGraphicsPipeline(device.CreateMaterial(
                Shader::Create("Assets/Shaders/EnginePreview.shader"), "EditorEnginePreview"),
                VertexBuffer::Create(vertices, sizeof(vertices)));
            RenderGraph::AttachmentSpecification spec;
            spec.ColorFormats = {FRAME_BUFFER_TEXTURE_FORMAT_RGBA8};
            m_Target = m_Graph.CreateRenderTarget("EditorPreview", spec, Width, Height);
            m_Readback = device.CreateTexture2DArray(Width, Height, 1, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
            std::vector<std::uint8_t> initial(Width * Height * 4);
            m_Readback->SetData(initial.data(), static_cast<uint32_t>(initial.size()), 0);
            m_Graph.BuildGraphicsPass("Editor preview triangle", m_Target,
                [this](const Ref<CommandBuffer>& command) { command->Render(m_Pipeline, 0); });
            const auto target = m_Graph.GetColorAttachment(m_Target);
            const auto array = m_Graph.ImportTexture("EditorReadback", m_Readback, RenderGraph::ResourceState::ShaderRead);
            const auto copy = m_Graph.BuildGraphicsCopyPass("Editor preview readback",
                [this](const Ref<CommandBuffer>& command)
                {
                    Graphics::GetRenderDevice().CopyTexture2DToArrayLayer(command,
                        m_Graph.GetFrameBuffer(m_Target)->GetRenderTarget(0), m_Readback, 0);
                }).GetHandle();
            m_Graph.Read(copy, target, RenderGraph::ResourceState::CopySource);
            m_Graph.Write(copy, array, RenderGraph::ResourceState::CopyDestination);
            GE_INFO("Editor engine preview initialized: backend={}, {}x{} RGBA8", static_cast<int>(Graphics::GetGraphicsAPI()), Width, Height);
        }
        void OnUpdate() override
        {
            m_Recorded = false;
            DWORD available = 0;
            if (!PeekNamedPipe(GetStdHandle(STD_INPUT_HANDLE), nullptr, 0, nullptr, &available, nullptr))
            { m_Runtime.Close(); return; }
            if (!available) return;
            std::uint8_t input[4096];
            const int count = _read(_fileno(stdin), input, static_cast<unsigned>(std::min<DWORD>(available, sizeof(input))));
            std::vector<Packet> packets;
            if (count <= 0 || !decoder.Feed(input, count, packets)) throw std::runtime_error("Invalid editor input.");
            for (const auto& packet : packets)
            {
                if (packet.Session != session || packet.Request <= lastRequest || !packet.Payload.empty())
                    throw std::runtime_error("Invalid editor control packet.");
                lastRequest = packet.Request;
                switch (packet.Type)
                {
                case Kind::Ping: Send(Kind::Pong, packet.Request); m_Render = true; break;
                case Kind::Shutdown: Send(Kind::Ack, packet.Request); m_Runtime.Close(); m_Render = false; break;
                case Kind::Crash: RaiseException(EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE, 0, nullptr); break;
                case Kind::Hang: for (;;) std::this_thread::sleep_for(std::chrono::seconds(60));
                case Kind::Malformed:
                {
                    const char invalid[32] = "invalid engine packet";
                    fwrite(invalid, 1, sizeof(invalid), output); fflush(output); break;
                }
                default: throw std::runtime_error("Unsupported editor control.");
                }
            }
        }
        void OnRender() override
        {
            if (m_Render)
            {
                m_Graph.ExecuteGpu(GEngine::GraphicsPresent::GetCommandBuffer());
                m_Recorded = true;
            }
        }
        void OnEndFrame() override
        {
            if (!m_Render || !m_Recorded) return;
            m_Render = false;
            std::vector<std::uint8_t> pixels(Width * Height * 4);
            // Bounded preview fallback: existing synchronous readback waits inside the child only.
            m_Readback->ReadLayerData(0, static_cast<uint32_t>(pixels.size()), pixels.data());
            std::vector<std::uint8_t> image;
            Append(image, Width, 4); Append(image, Height, 4); Append(image, Width * 4, 4);
            const bool flip = GEngine::Graphics::GetGraphicsAPI() == GEngine::GRAPHICS_API_OPENGL;
            for (unsigned y = 0; y < Height; ++y)
            {
                const auto row = pixels.begin() + (flip ? Height - y - 1 : y) * Width * 4;
                image.insert(image.end(), row, row + Width * 4);
            }
            Send(Kind::EngineFrame, 0, std::move(image));
            if (++m_Frames == 1 || m_Frames % 20 == 0) GE_INFO("Editor engine frame {} delivered", m_Frames);
        }
    private:
        static constexpr unsigned Width = 160, Height = 90;
        GEngine::Application& m_Runtime;
        GEngine::RenderGraph m_Graph;
        GEngine::RenderGraph::TargetHandle m_Target = 0;
        GEngine::Ref<GEngine::GraphicsPipeline> m_Pipeline;
        GEngine::Ref<GEngine::Texture2DArray> m_Readback;
        bool m_Render = true;
        bool m_Recorded = false;
        unsigned m_Frames = 0;
    };
}

int main(int, char**)
{
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter([](EXCEPTION_POINTERS*) -> LONG { return EXCEPTION_EXECUTE_HANDLER; });
    if (_setmode(_fileno(stdin), _O_BINARY) == -1 || _setmode(_fileno(stdout), _O_BINARY) == -1) return 1;
    const int protocol = _dup(_fileno(stdout));
    if (protocol == -1) return 1;
    output = _fdopen(protocol, "wb");
    // Reserve the original pipe for packets; all engine stdout logs go to stderr.
    if (!output || _dup2(_fileno(stderr), _fileno(stdout)) != 0) return 1;
    setvbuf(stdout, nullptr, _IONBF, 0);
    try
    {
        Packet hello;
        for (;;)
        {
            std::uint8_t input[4096];
            const int count = _read(_fileno(stdin), input, sizeof(input));
            std::vector<Packet> packets;
            if (count <= 0 || !decoder.Feed(input, count, packets)) return 2;
            if (packets.empty()) continue;
            if (packets.size() != 1) return 2;
            hello = std::move(packets.front()); break;
        }
        if (hello.Type != Kind::Hello || Text(hello.Payload) != BuildId || !hello.Request) return 3;
        session = hello.Session; lastRequest = hello.Request;
        GEngine::RuntimeSpecification spec;
        spec.Name = "GEngine editor render worker"; spec.Size = {640, 360};
        spec.EnableImGui = spec.EnableScripting = spec.EnablePhysics = spec.EnableOpenSSL = false;
        {
            // Shader cache still uses the Application compatibility singleton.
            GEngine::Application runtime(spec);
            // The worker owns its native presentation surface; Qt displays only readback frames.
            EnumThreadWindows(GetCurrentThreadId(), [](HWND window, LPARAM) -> BOOL
            { ShowWindow(window, SW_HIDE); return TRUE; }, 0);
            runtime.PushLayer(new Preview(runtime));
            Send(Kind::Ready, hello.Request, Text(EngineBuildId));
            runtime.Run();
        }
        fclose(output); return 0;
    }
    catch (const std::exception& error)
    { std::cerr << "EngineWorker failed: " << error.what() << std::endl; fclose(output); return 1; }
}
