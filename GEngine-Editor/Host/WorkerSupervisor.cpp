#include "WorkerSupervisor.h"
#include <QFileInfo>
#include <algorithm>

namespace GEngineEditor
{
    using namespace Protocol;
    WorkerSupervisor::WorkerSupervisor(QObject* parent) : QObject(parent)
    {
        m_Clock.start();
        m_Timer.setInterval(16);
        connect(&m_Timer, &QTimer::timeout, this, [this] { Tick(); });
        m_Timer.start();
        connect(&m_Process, &QProcess::started, this, [this]
        {
            Packet hello{Kind::Hello, m_Session, ++m_Request, Text(BuildId)};
            m_HelloRequest = hello.Request;
            const auto bytes = Encode(hello);
            m_Process.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        });
        connect(&m_Process, &QProcess::readyReadStandardOutput, this, [this] { ReadOutput(); });
        connect(&m_Process, &QProcess::readyReadStandardError, this, [this]
        { ReadError(); });
        connect(&m_Process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error)
        {
            Log(m_Process.errorString());
            if (error == QProcess::FailedToStart) SetState(State::Disconnected);
        });
        connect(&m_Process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this](int code, QProcess::ExitStatus)
        {
            ReadOutput();
            ReadError(true);
            if (OnConsoleReset) OnConsoleReset();
            if (m_Decoder.HasPartial()) Log("Discarded truncated protocol packet on exit.");
            Log(QString("Worker exited with code %1.").arg(code));
            m_Decoder.Reset();
            SetState(m_ExpectedStop ? State::Stopped : State::Disconnected);
        });
    }
    WorkerSupervisor::~WorkerSupervisor()
    {
        // Final shutdown fallback only; normal UI stop uses the asynchronous deadline.
        if (Running()) { m_Process.kill(); m_Process.waitForFinished(2000); }
    }
    void WorkerSupervisor::Log(const QString& text) { if (OnLog) OnLog(text); }
    void WorkerSupervisor::SetState(State state)
    {
        m_State = state;
        if (OnState) OnState(state);
    }
    QString WorkerSupervisor::Description() const
    {
        switch (m_State)
        {
        case State::Stopped: return "Stopped";
        case State::Starting: return "Starting";
        case State::Ready: return m_IsEngine ? "Ready (live engine scene)" : "Ready (protocol diagnostic only)";
        case State::Unresponsive: return "Unresponsive - stop/restart available";
        case State::Stopping: return "Stopping";
        case State::Disconnected: return "Disconnected - document remains editable";
        }
        return "Unknown";
    }
    void WorkerSupervisor::SetViewportSize(QSize size)
    {
        if (size.isEmpty()) { m_ViewportSize = {}; return; }
        if (size.width() > static_cast<int>(MaxFrameWidth) || size.height() > static_cast<int>(MaxFrameHeight))
            size.scale(MaxFrameWidth, MaxFrameHeight, Qt::KeepAspectRatio);
        m_ViewportSize = size.expandedTo(QSize(1, 1));
    }
    void WorkerSupervisor::Start(const QString& executable, const QString& scene)
    {
        if (Running()) return;
        if (OnConsoleReset) OnConsoleReset();
        m_Decoder.Reset();
        m_IsEngine = false;
        m_ReceivedFrame = false;
        m_FrameRequest = 0; m_FrameBegun = false; m_FramePixels.clear(); m_NextFrame = 0; m_LastFrameSize = {};
        m_ExpectedStop = false;
        m_Request = m_PingRequest = 0;
        if (++m_Session == 0) ++m_Session;
        m_Deadline = m_Clock.elapsed() + 30000;
        m_Process.setProgram(QFileInfo(executable).absoluteFilePath());
        m_Process.setArguments(scene.isEmpty() ? QStringList{} : QStringList{"--scene", scene});
        m_Process.setWorkingDirectory(QFileInfo(executable).absolutePath());
        SetState(State::Starting);
        m_Process.start();
    }
    void WorkerSupervisor::Send(Kind kind, std::vector<std::uint8_t> payload)
    {
        if (!Running()) return;
        if (m_Process.bytesToWrite() > 2 * static_cast<qint64>(MaxPayload))
        { Fail("Outgoing protocol queue limit exceeded."); return; }
        Packet packet{kind, m_Session, ++m_Request, std::move(payload)};
        if (kind == Kind::Ping) m_PingRequest = packet.Request;
        if (kind == Kind::RequestFrame) m_FrameRequest = packet.Request;
        auto bytes = Encode(packet);
        if (m_Process.write(reinterpret_cast<const char*>(bytes.data()), bytes.size()) < 0)
            Fail("Protocol write failed.");
    }
    void WorkerSupervisor::Stop()
    {
        if (!Running()) { SetState(State::Stopped); return; }
        m_ExpectedStop = true;
        m_Deadline = m_Clock.elapsed() + 1500;
        const auto previous = m_State;
        SetState(State::Stopping);
        if (previous == State::Ready) Send(Kind::Shutdown);
        else m_Process.kill();
    }
    void WorkerSupervisor::Inject(Kind kind)
    {
        if (m_State != State::Ready) return;
        if (kind == Kind::Crash || kind == Kind::Hang || kind == Kind::Malformed) Send(kind);
    }
    void WorkerSupervisor::Fail(const QString& reason)
    {
        Log(reason);
        m_ExpectedStop = false;
        SetState(State::Disconnected);
        if (Running()) m_Process.kill();
    }
    void WorkerSupervisor::ReadOutput()
    {
        m_Process.setReadChannel(QProcess::StandardOutput);
        if (!m_Process.bytesAvailable()) return;
        const auto bytes = m_Process.read(MaxPayload);
        std::vector<Packet> packets;
        if (!m_Decoder.Feed(reinterpret_cast<const std::uint8_t*>(bytes.constData()), bytes.size(), packets))
        { Fail(QString::fromStdString(m_Decoder.Error())); return; }
        for (const auto& packet : packets)
        {
            Handle(packet);
            if (m_State == State::Disconnected) return;
        }
        if (m_Process.bytesAvailable()) QTimer::singleShot(0, this, [this] { ReadOutput(); });
    }
    void WorkerSupervisor::ReadError(bool drain)
    {
        m_Process.setReadChannel(QProcess::StandardError);
        do
        {
            const auto bytes = m_Process.read(16 * 1024);
            if (!bytes.isEmpty() && OnConsoleOutput) OnConsoleOutput(bytes);
        } while (drain && m_Process.bytesAvailable());
        if (m_Process.bytesAvailable()) QTimer::singleShot(0, this, [this] { ReadError(); });
    }
    void WorkerSupervisor::Handle(const Packet& packet)
    {
        if (packet.Session != m_Session) { Log("Discarded old-session packet."); return; }
        // Replies already in flight are harmless while the owned process is shutting down.
        if (m_State == State::Stopping) return;
        if (m_IsEngine && m_State == State::Ready &&
            (packet.Type == Kind::FrameBegin || packet.Type == Kind::FrameChunk || packet.Type == Kind::FrameEnd))
        {
            if (!m_FrameRequest || packet.Request != m_FrameRequest)
            { Fail("Unexpected scene frame generation."); return; }
            const auto expected = static_cast<std::size_t>(m_RequestedSize.width()) * m_RequestedSize.height() * 4;
            if (packet.Type == Kind::FrameBegin)
            {
                const auto& data = packet.Payload;
                if (m_FrameBegun || data.size() != 12 ||
                    Read(data.data(), 4) != static_cast<unsigned>(m_RequestedSize.width()) ||
                    Read(data.data() + 4, 4) != static_cast<unsigned>(m_RequestedSize.height()) ||
                    Read(data.data() + 8, 4) != static_cast<unsigned>(m_RequestedSize.width() * 4))
                { Fail("Invalid scene frame header."); return; }
                m_FrameBegun = true; m_FramePixels.clear(); m_FramePixels.reserve(expected);
            }
            else if (packet.Type == Kind::FrameChunk)
            {
                if (!m_FrameBegun || packet.Payload.empty() || packet.Payload.size() > expected - m_FramePixels.size())
                { Fail("Invalid scene frame chunk."); return; }
                m_FramePixels.insert(m_FramePixels.end(), packet.Payload.begin(), packet.Payload.end());
            }
            else
            {
                if (!m_FrameBegun || !packet.Payload.empty() || m_FramePixels.size() != expected)
                { Fail("Incomplete scene frame."); return; }
                const QImage image(m_FramePixels.data(), m_RequestedSize.width(), m_RequestedSize.height(),
                    m_RequestedSize.width() * 4, QImage::Format_RGBA8888);
                const auto owned = image.copy();
                if (owned.isNull()) { Fail("Could not allocate scene frame."); return; }
                m_FrameRequest = 0; m_FrameBegun = false; m_FramePixels.clear(); m_ReceivedFrame = true;
                if (m_RequestedSize == m_ViewportSize)
                {
                    m_LastFrameSize = m_RequestedSize;
                    if (OnFrame) OnFrame(owned);
                }
            }
            return;
        }
        if (packet.Type == Kind::Ready && m_State == State::Starting &&
            packet.Request == m_HelloRequest &&
            (Text(packet.Payload) == BuildId || Text(packet.Payload) == EngineBuildId))
        {
            m_IsEngine = Text(packet.Payload) == EngineBuildId;
            m_NextPing = m_Clock.elapsed();
            SetState(State::Ready);
            return;
        }
        if (packet.Type == Kind::Pong && m_State == State::Ready &&
            packet.Request == m_PingRequest && m_PingRequest != 0 && packet.Payload.empty())
        {
            m_PingRequest = 0;
            m_NextPing = m_Clock.elapsed() + 500;
            return;
        }
        if (!m_IsEngine && packet.Type == Kind::DiagnosticFrame &&
            m_State == State::Ready && packet.Request == 0)
        {
            const auto& data = packet.Payload;
            if (data.size() < 12) { Fail("Truncated diagnostic image."); return; }
            const auto width = Read(data.data(), 4);
            const auto height = Read(data.data() + 4, 4);
            const auto stride = Read(data.data() + 8, 4);
            if (!width || !height || width > 256 || height > 256 || stride != width * 4 ||
                stride * height != data.size() - 12)
            { Fail("Invalid diagnostic image dimensions."); return; }
            QImage image(data.data() + 12, static_cast<int>(width), static_cast<int>(height),
                static_cast<int>(stride), QImage::Format_RGBA8888);
            const auto ownedImage = image.copy();
            if (ownedImage.isNull()) { Fail("Could not allocate diagnostic image."); return; }
            m_ReceivedFrame = true;
            if (OnFrame) OnFrame(ownedImage);
            return;
        }
        Fail("Unexpected diagnostic protocol response.");
    }
    void WorkerSupervisor::Tick()
    {
        const auto now = m_Clock.elapsed();
        if (m_State == State::Starting && now > m_Deadline) Fail("Worker handshake timed out.");
        else if (m_State == State::Stopping && now > m_Deadline && Running()) m_Process.kill();
        else if (m_State == State::Ready)
        {
            if (m_IsEngine && m_FrameRequest && now > m_FrameDeadline)
            { Log("Scene frame timed out."); SetState(State::Unresponsive); return; }
            if (m_IsEngine && !m_FrameRequest && !m_ViewportSize.isEmpty() && now >= m_NextFrame)
            {
                m_RequestedSize = m_ViewportSize;
                std::vector<std::uint8_t> payload;
                Append(payload, m_RequestedSize.width(), 4); Append(payload, m_RequestedSize.height(), 4);
                m_FrameDeadline = now + 10000; m_NextFrame = now + 33;
                Send(Kind::RequestFrame, std::move(payload));
            }
            if (m_PingRequest && now > m_Deadline)
            { Log("Worker heartbeat timed out."); SetState(State::Unresponsive); }
            else if (!m_PingRequest && now >= m_NextPing)
            { m_Deadline = now + (m_IsEngine && !m_ReceivedFrame ? 10000 : 2000); Send(Kind::Ping); }
        }
    }
}
