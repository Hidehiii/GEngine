#pragma once
#include "Core/DraftDocument.h"
#include "WorkerSupervisor.h"
#include <QMainWindow>
#include <QImage>

class QPlainTextEdit;
class QLabel;
class QAction;

namespace GEngineEditor
{
    class ViewportWidget;
    class MainWindow : public QMainWindow
    {
    public:
        explicit MainWindow(QString workerPath);
        ~MainWindow() override;
        WorkerSupervisor& Worker() { return m_Worker; }
        DraftDocument& Document() { return m_Document; }
        void SetDraftText(const QString& text);
        bool SaveDraft(const QString& path);
        bool OpenDraft(const QString& path);
        bool HasImage() const { return m_HasImage; }
        unsigned FrameCount() const { return m_FrameCount; }
        bool HasEngineEvidence() const;
        QString LastError() const { return m_Error; }
        void StartWorker();
    protected:
        void closeEvent(QCloseEvent* event) override;
    private:
        void RefreshDocument();
        void RefreshWorker();
        bool ConfirmDiscard();
        bool SaveAs();
        void Log(const QString& text);
        void ResetLayout();
        DraftDocument m_Document;
        WorkerSupervisor m_Worker;
        QString m_WorkerPath;
        QString m_DraftPath;
        QString m_Error;
        QPlainTextEdit* m_Notes = nullptr;
        QPlainTextEdit* m_Console = nullptr;
        QLabel* m_StateLabel = nullptr;
        ViewportWidget* m_Viewport = nullptr;
        QAction* m_Undo = nullptr;
        QAction* m_Redo = nullptr;
        QAction* m_Start = nullptr;
        QAction* m_Stop = nullptr;
        QAction* m_Crash = nullptr;
        QAction* m_Hang = nullptr;
        QAction* m_Malformed = nullptr;
        bool m_HasImage = false;
        unsigned m_FrameCount = 0;
        bool m_ClosePending = false;
    };
    void RunRecoveryCheck(MainWindow& window);
}
