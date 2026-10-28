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
#include <cmath>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <GEngine/Tools/Serializer.h>
#include <GEngine/Tools/FileSystemHelper.h>
#include <GEngine/Renderer/SceneRenderer.h>
#include <spdlog/sinks/ansicolor_sink.h>

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
    std::string scenePath;
    class SceneViewport final : public GEngine::Layer
    {
    public:
        explicit SceneViewport(GEngine::Application& runtime) : m_Runtime(runtime) {}
        void OnAttach() override
        {
            using namespace GEngine;
            m_Scene = CreateRef<Scene>();
            if (!scenePath.empty())
            {
                if (std::filesystem::file_size(std::filesystem::u8path(scenePath)) > 4 * 1024 * 1024)
                    throw std::runtime_error("Scene file exceeds 4 MiB.");
                const auto data = YAML::Load(FileSystemHelper::ReadFileAsString(scenePath));
                if (!data["Scene"]) throw std::runtime_error("Missing Scene document.");
                // The legacy serializer cannot round-trip materials, meshes or camera rendering yet.
                for (const auto& object : data["GameObjects"])
                {
                    for (const auto& field : object)
                    {
                        const auto key = field.first.as<std::string>();
                        if (key != "GameObject" && key != "Name" && key != "Attribute" &&
                            key != "Transform" && key != "ImageRenderer")
                            throw std::runtime_error("Unsupported scene component: " + key);
                    }
                    if (object["ImageRenderer"] && object["ImageRenderer"]["Texture"].as<std::string>() != "None")
                        throw std::runtime_error("Textured scene rendering is unsupported.");
                }
                Serializer::Deserialize(scenePath, m_Scene);
                for (auto object : m_Scene->GetGameObjects<ImageRenderer>())
                    SetMaterial(object.GetComponent<ImageRenderer>());
                GE_INFO("Editor scene loaded: {}", m_Scene->name);
            }
            else
            {
                m_Scene->name = "Engine sample scene";
                auto left = m_Scene->CreateGameObject("Red quad");
                left.GetComponent<Transform>().SetPosition({-0.5f, 0.0f, 0.0f});
                left.GetComponent<Transform>().SetScale({0.6f, 0.7f, 1.0f});
                left.AddComponent<ImageRenderer>(Vector4{0.95f, 0.22f, 0.18f, 1.0f});
                SetMaterial(left.GetComponent<ImageRenderer>());
                m_Animated = m_Scene->CreateGameObject("Moving cyan quad");
                m_Animated.GetComponent<Transform>().SetPosition({0.5f, 0.0f, 0.0f});
                m_Animated.GetComponent<Transform>().SetScale({0.6f, 0.5f, 1.0f});
                m_Animated.AddComponent<ImageRenderer>(Vector4{0.1f, 0.75f, 1.0f, 1.0f});
                SetMaterial(m_Animated.GetComponent<ImageRenderer>());
            }
            GE_INFO("Editor SceneRenderer initialized: backend={}, scene={}",
                static_cast<int>(Graphics::GetGraphicsAPI()), m_Scene->name);
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
                if (packet.Session != session || packet.Request <= lastRequest ||
                    (packet.Type != Kind::RequestFrame && !packet.Payload.empty()))
                    throw std::runtime_error("Invalid editor control packet.");
                lastRequest = packet.Request;
                switch (packet.Type)
                {
                case Kind::Ping: Send(Kind::Pong, packet.Request); break;
                case Kind::RequestFrame:
                {
                    if (packet.Payload.size() != 8 || m_FrameRequest) throw std::runtime_error("Invalid frame request.");
                    const auto width = Read(packet.Payload.data(), 4), height = Read(packet.Payload.data() + 4, 4);
                    if (!width || !height || width > MaxFrameWidth || height > MaxFrameHeight)
                        throw std::runtime_error("Invalid viewport dimensions.");
                    if (width != m_Width || height != m_Height) Resize(static_cast<unsigned>(width), static_cast<unsigned>(height));
                    m_FrameRequest = packet.Request;
                    if (scenePath.empty())
                    {
                        const auto seconds = std::chrono::duration<float>(std::chrono::steady_clock::now() - m_Start).count();
                        m_Animated.GetComponent<GEngine::Transform>().SetPosition({0.5f, 0.25f * std::sin(seconds * 2.0f), 0.0f});
                    }
                    break;
                }
                case Kind::Shutdown: Send(Kind::Ack, packet.Request); m_Runtime.Close(); m_FrameRequest = 0; break;
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
            if (m_FrameRequest)
            {
                m_Graph.ExecuteGpu(GEngine::GraphicsPresent::GetCommandBuffer());
                m_Recorded = true;
            }
        }
        void OnEndFrame() override
        {
            if (!m_FrameRequest || !m_Recorded) return;
            std::vector<std::uint8_t> pixels(m_Width * m_Height * 4);
            // Synchronous fallback is confined to the child; one credit bounds pipe/memory growth.
            m_Readback->ReadLayerData(0, static_cast<uint32_t>(pixels.size()), pixels.data());
            if (GEngine::Graphics::GetGraphicsAPI() == GEngine::GRAPHICS_API_OPENGL)
                for (unsigned y = 0; y < m_Height / 2; ++y)
                    std::swap_ranges(pixels.begin() + y * m_Width * 4, pixels.begin() + (y + 1) * m_Width * 4,
                        pixels.begin() + (m_Height - y - 1) * m_Width * 4);
            std::vector<std::uint8_t> header;
            Append(header, m_Width, 4); Append(header, m_Height, 4); Append(header, m_Width * 4, 4);
            Send(Kind::FrameBegin, m_FrameRequest, std::move(header));
            for (std::size_t offset = 0; offset < pixels.size(); offset += MaxPayload)
                Send(Kind::FrameChunk, m_FrameRequest, {pixels.begin() + offset,
                    pixels.begin() + std::min(pixels.size(), offset + MaxPayload)});
            Send(Kind::FrameEnd, m_FrameRequest);
            m_FrameRequest = 0;
            m_Scene->OnEndFrame();
            if (++m_Frames == 1 || m_Frames % 300 == 0) GE_INFO("Editor engine frame {} delivered", m_Frames);
        }
    private:
        void SetMaterial(GEngine::ImageRenderer& image)
        {
            image.SetMaterial(GEngine::Graphics::GetRenderDevice().CreateMaterial(
                GEngine::Shader::Create("Assets/Shaders/SceneImageColor.shader"), "Scene image"));
        }
        void Resize(unsigned width, unsigned height)
        {
            using namespace GEngine;
            // Previous frame readback completed before another frame credit can reach this path.
            m_Graph.Reset();
            m_Readback.reset();
            m_Width = width; m_Height = height;
            m_Scene->OnViewportResize(width, height);
            auto& device = Graphics::GetRenderDevice();
            RenderGraph::AttachmentSpecification spec;
            spec.ColorFormats = {FRAME_BUFFER_TEXTURE_FORMAT_RGBA8};
            m_Target = m_Graph.CreateRenderTarget("EditorScene", spec, width, height);
            m_Readback = device.CreateTexture2DArray(width, height, 1, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM);
            std::vector<std::uint8_t> initial(width * height * 4);
            m_Readback->SetData(initial.data(), static_cast<uint32_t>(initial.size()), 0);
            m_Graph.BuildGraphicsPass("Editor scene", m_Target,
                [this](const Ref<CommandBuffer>& command) { SceneRenderer::Render(*m_Scene, command); });
            const auto target = m_Graph.GetColorAttachment(m_Target);
            const auto array = m_Graph.ImportTexture("EditorReadback", m_Readback, RenderGraph::ResourceState::ShaderRead);
            const auto copy = m_Graph.BuildGraphicsCopyPass("Editor scene readback",
                [this](const Ref<CommandBuffer>& command)
                {
                    Graphics::GetRenderDevice().CopyTexture2DToArrayLayer(command,
                        m_Graph.GetFrameBuffer(m_Target)->GetRenderTarget(0), m_Readback, 0);
                }).GetHandle();
            m_Graph.Read(copy, target, RenderGraph::ResourceState::CopySource);
            m_Graph.Write(copy, array, RenderGraph::ResourceState::CopyDestination);
            GE_INFO("Editor scene target {}x{}", width, height);
        }
        GEngine::Application& m_Runtime;
        GEngine::Ref<GEngine::Scene> m_Scene;
        GEngine::GameObject m_Animated;
        GEngine::RenderGraph m_Graph;
        GEngine::RenderGraph::TargetHandle m_Target = 0;
        GEngine::Ref<GEngine::Texture2DArray> m_Readback;
        unsigned m_Width = 0, m_Height = 0, m_Frames = 0;
        std::uint64_t m_FrameRequest = 0;
        bool m_Recorded = false;
        std::chrono::steady_clock::time_point m_Start = std::chrono::steady_clock::now();
    };
}

int wmain(int argc, wchar_t** argv)
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
        if (argc == 3 && std::wstring(argv[1]) == L"--scene") scenePath = std::filesystem::path(argv[2]).u8string();
        else if (argc != 1) throw std::runtime_error("Expected --scene <absolute-path>.");
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
        // Windows console attributes are lost in a pipe; preserve the original level palette as SGR.
        GEngine::Log::Init();
        auto colors = std::make_shared<spdlog::sinks::ansicolor_stderr_sink_mt>(spdlog::color_mode::always);
        colors->set_pattern("%^[%T] %n: %v%$");
        colors->set_color(spdlog::level::trace, "\x1b[37m");
        colors->set_color(spdlog::level::debug, "\x1b[36m");
        colors->set_color(spdlog::level::info, "\x1b[32m");
        colors->set_color(spdlog::level::warn, "\x1b[93m");
        colors->set_color(spdlog::level::err, "\x1b[91m");
        colors->set_color(spdlog::level::critical, "\x1b[97;41m");
        GEngine::Log::GetCoreLogger()->sinks().front() = colors;
        GEngine::Log::GetClientLogger()->sinks().front() = colors;
        GEngine::RuntimeSpecification spec;
        spec.Name = "GEngine editor render worker"; spec.Size = {640, 360};
        spec.EnableImGui = spec.EnableScripting = spec.EnablePhysics = spec.EnableOpenSSL = false;
        {
            // Shader cache still uses the Application compatibility singleton.
            GEngine::Application runtime(spec);
            // The worker owns its native presentation surface; Qt displays only readback frames.
            EnumThreadWindows(GetCurrentThreadId(), [](HWND window, LPARAM) -> BOOL
            { ShowWindow(window, SW_HIDE); return TRUE; }, 0);
            runtime.PushLayer(new SceneViewport(runtime));
            Send(Kind::Ready, hello.Request, Text(EngineBuildId));
            runtime.Run();
        }
        fclose(output); return 0;
    }
    catch (const std::exception& error)
    { std::cerr << "EngineWorker failed: " << error.what() << std::endl; fclose(output); return 1; }
}
