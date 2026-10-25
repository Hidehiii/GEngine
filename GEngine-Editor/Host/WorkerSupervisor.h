#pragma once
#include "Protocol/EditorProtocol.h"
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QElapsedTimer>
#include <QImage>
#include <functional>

namespace GEngineEditor
{
    class WorkerSupervisor : public QObject
    {
    public:
        enum class State { Stopped, Starting, Ready, Unresponsive, Stopping, Disconnected };
        explicit WorkerSupervisor(QObject* parent = nullptr);
        ~WorkerSupervisor() override;
        void Start(const QString& executable);
        void Stop();
        void Inject(Protocol::Kind kind);
        State GetState() const { return m_State; }
        bool Running() const { return m_Process.state() != QProcess::NotRunning; }
        QString Description() const;
        bool IsEngine() const { return m_IsEngine; }
        std::function<void(State)> OnState;
        std::function<void(const QString&)> OnLog;
        std::function<void(const QImage&)> OnFrame;
    private:
        void SetState(State state);
        void Send(Protocol::Kind kind);
        void ReadOutput();
        void ReadError();
        void Handle(const Protocol::Packet& packet);
        void Fail(const QString& reason);
        void Tick();
        void Log(const QString& value);
        QProcess m_Process;
        QTimer m_Timer;
        QElapsedTimer m_Clock;
        Protocol::Decoder m_Decoder;
        State m_State = State::Stopped;
        std::uint64_t m_Session = 0;
        std::uint64_t m_Request = 0;
        std::uint64_t m_HelloRequest = 0;
        std::uint64_t m_PingRequest = 0;
        qint64 m_Deadline = 0;
        qint64 m_NextPing = 0;
        bool m_ExpectedStop = false;
        bool m_IsEngine = false;
        bool m_ReceivedFrame = false;
    };
}
