#pragma once
#include "Core/DraftDocument.h"
#include "WorkerSupervisor.h"
#include "ProjectWorkspace.h"
#include "ProjectFile.h"
#include <QMainWindow>
#include <QImage>
#include <unordered_map>
#include <memory>

class QPlainTextEdit;
class QLabel;
class QAction;
class QTabWidget;
class QTabBar;
class QDockWidget;
class QMenu;
class QMenuBar;
class QProcess;
class QTimer;

namespace GEngineEditor
{
    class ViewportWidget;
    class ConsoleOutput;
    class MainWindow : public QMainWindow
    {
    public:
        explicit MainWindow(QString workerPath);
        ~MainWindow() override;
        WorkerSupervisor& Worker() { return m_Worker; }
        DraftDocument& Document();
        bool NewDraft();
        bool CloseDraft(int index);
        int DraftCount() const;
        void SetDraftText(const QString& text);
        bool SaveDraft(const QString& path);
        bool OpenDraft(const QString& path);
        bool HasImage() const { return m_HasImage; }
        unsigned FrameCount() const { return m_FrameCount; }
        bool HasEngineEvidence() const;
        QString LastError() const { return m_Error; }
        void StartWorker();
        bool OpenProject(const QString& path);
        void CloseProject();
        ProjectWorkspace& Workspace() { return *m_Workspace; }
        QString InspectorText() const;
        bool NewProject(const QString& path);
        bool SavePackaging(const QString& output);
        bool PackageProject();
        void CancelPackaging();
        bool PackagingRunning() const;
        bool LastPackagingSucceeded() const { return m_PackagingSucceeded; }
        QString ProjectPath() const { return m_Project.Path; }
    protected:
        bool eventFilter(QObject* watched, QEvent* event) override;
        void closeEvent(QCloseEvent* event) override;
    private:
        void RefreshDocument();
        void RefreshWorker();
        bool ConfirmDiscard();
        bool ConfirmAllDrafts();
        bool SaveAs();
        void Log(const QString& text);
        void ResetLayout();
        void RefreshDockTabs();
        void InitProjectActions(QMenu* file, QMenuBar* menus);
        void RefreshPackagingActions();
        bool ShowPackagingDialog();
        QDockWidget* DockForTab(QTabBar* bar, int index) const;
        struct DraftTab
        {
            DraftDocument Document;
            QString Path;
            QPlainTextEdit* Editor = nullptr;
            unsigned Untitled = 0;
        };
        DraftTab& ActiveDraft();
        std::unordered_map<QWidget*, std::unique_ptr<DraftTab>> m_Drafts;
        QTabWidget* m_DraftTabs = nullptr;
        QTabBar* m_BrowserTabs = nullptr;
        bool m_DockTabsUpdating = false;
        bool m_DockTabsUpdatePending = false;
        unsigned m_NextUntitled = 0;
        WorkerSupervisor m_Worker;
        QString m_WorkerPath;
        QString m_Error;
        ConsoleOutput* m_Console = nullptr;
        QLabel* m_StateLabel = nullptr;
        ViewportWidget* m_Viewport = nullptr;
        QAction* m_Undo = nullptr;
        QAction* m_Redo = nullptr;
        QAction* m_Start = nullptr;
        QAction* m_Stop = nullptr;
        QAction* m_Crash = nullptr;
        QAction* m_Hang = nullptr;
        QAction* m_Malformed = nullptr;
        ProjectWorkspace* m_Workspace = nullptr;
        QPlainTextEdit* m_Inspector = nullptr;
        QAction* m_CloseProject = nullptr;
        ProjectFile m_Project;
        QString m_PackageOutputDirectory;
        QProcess* m_PackagingProcess = nullptr;
        QTimer* m_PackagingDeadline = nullptr;
        QAction* m_Package = nullptr;
        QAction* m_CancelPackaging = nullptr;
        bool m_PackagingSucceeded = false;
        bool m_PackagingCancelled = false;
        QByteArray m_PackagingOutput;
        bool m_HasImage = false;
        unsigned m_FrameCount = 0;
        bool m_ClosePending = false;
        bool m_ConfirmingClose = false;
    };
    void RunRecoveryCheck(MainWindow& window);
    void RunPackagingCheck(MainWindow& window);
}
