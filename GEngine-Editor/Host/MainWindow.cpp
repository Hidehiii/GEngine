#include "MainWindow.h"
#include <QTemporaryFile>
#include "ConsoleOutput.h"
#include <QAction>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDockWidget>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QSaveFile>
#include <QSettings>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTabBar>
#include <QDir>
#include <QScopedValueRollback>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QApplication>
#include <QEvent>
#include <QPointer>
#include <QProcess>
#include <QCryptographicHash>
#include <iostream>

namespace GEngineEditor
{
    class ViewportWidget : public QWidget
    {
    public:
        QImage Image;
        QString State;
        bool Engine = false;
        QString Scene = "Engine sample scene";
        QString EmptyMessage;
        std::function<void(QSize)> OnSize;
        explicit ViewportWidget(QWidget* parent = nullptr) : QWidget(parent) { setMinimumSize(320, 240); }
    protected:
        bool event(QEvent* event) override
        {
            const auto result = QWidget::event(event);
            if (OnSize && (event->type() == QEvent::Resize || event->type() == QEvent::Show ||
                event->type() == QEvent::Hide || event->type() == QEvent::DevicePixelRatioChange))
                OnSize(isVisible() ? QSize(qRound(width() * devicePixelRatioF()), qRound(height() * devicePixelRatioF())) : QSize{});
            return result;
        }
        void paintEvent(QPaintEvent*) override
        {
            QPainter painter(this);
            painter.fillRect(rect(), QColor(28, 30, 34));
            if (!Image.isNull())
            {
                const auto size = Image.size().scaled(this->size(), Qt::KeepAspectRatio);
                const QRect target((width() - size.width()) / 2, (height() - size.height()) / 2,
                    size.width(), size.height());
                painter.drawImage(target, Image);
            }
            painter.fillRect(0, 0, width(), 60, QColor(0, 0, 0, 200));
            painter.setPen(Qt::white);
            painter.drawText(rect().adjusted(12, 6, -12, -6), Qt::AlignTop | Qt::TextWordWrap,
                EmptyMessage.isEmpty()
                    ? (Engine ? "Live engine scene: " + Scene + "\n" : "CPU protocol diagnostic image - NOT an engine scene\n") + State
                    : EmptyMessage);
        }
    };
    MainWindow::MainWindow(QString workerPath) : m_WorkerPath(std::move(workerPath))
    {
        resize(1280, 800);
        setObjectName("EditorHost");
        setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
        qApp->installEventFilter(this);
        setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks |
            QMainWindow::AnimatedDocks | QMainWindow::GroupedDragging);
        auto* header = new QWidget(this);
        header->setObjectName("BrowserHeader");
        auto* headerLayout = new QVBoxLayout(header);
        headerLayout->setContentsMargins(0, 0, 0, 0); headerLayout->setSpacing(0);
        auto* strip = new QWidget(header);
        auto* stripLayout = new QHBoxLayout(strip);
        stripLayout->setContentsMargins(8, 4, 8, 0); stripLayout->setSpacing(4);
        m_BrowserTabs = new QTabBar(strip);
        m_BrowserTabs->setObjectName("BrowserDocumentTabs");
        m_BrowserTabs->setDocumentMode(true);
        m_BrowserTabs->setExpanding(false);
        m_BrowserTabs->setMovable(true);
        m_BrowserTabs->setTabsClosable(true);
        m_BrowserTabs->setUsesScrollButtons(true);
        m_BrowserTabs->setElideMode(Qt::ElideRight);
        m_BrowserTabs->setMinimumWidth(0);
        m_BrowserTabs->setStyleSheet(
            "QTabBar::tab { min-width: 100px; max-width: 220px; padding: 8px 12px; "
            "border: 1px solid palette(mid); border-bottom: none; border-top-left-radius: 8px; "
            "border-top-right-radius: 8px; background: palette(midlight); } "
            "QTabBar::tab:selected { background: palette(base); } "
            "QTabBar::tab:hover:!selected { background: palette(light); }");
        stripLayout->addWidget(m_BrowserTabs);
        auto* newTab = new QToolButton(strip);
        newTab->setObjectName("NewDocumentTabButton");
        newTab->setText("+"); newTab->setToolTip("New draft (Ctrl+T)");
        newTab->setFixedSize(32, 32); newTab->setAutoRaise(true);
        stripLayout->addWidget(newTab); stripLayout->addStretch();
        auto* menus = new QMenuBar(header);
        headerLayout->addWidget(menus);
        setMenuWidget(header);
        connect(newTab, &QToolButton::clicked, this, [this] { NewDraft(); });
        connect(m_BrowserTabs, &QTabBar::currentChanged, this, [this](int index)
        { if (m_DraftTabs && index >= 0 && index < DraftCount()) m_DraftTabs->setCurrentIndex(index); });
        connect(m_BrowserTabs, &QTabBar::tabCloseRequested, this, [this](int index) { CloseDraft(index); });
        connect(m_BrowserTabs, &QTabBar::tabMoved, this, [this](int from, int to)
        { m_DraftTabs->tabBar()->moveTab(from, to); RefreshDocument(); });
        auto* viewMenu = menus->addMenu("View");
        auto dock = [this, viewMenu](const QString& title, const char* id, QWidget* content, Qt::DockWidgetArea area)
        {
            auto* result = new QDockWidget(title, this);
            result->setObjectName(id);
            result->setAllowedAreas(Qt::AllDockWidgetAreas);
            result->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable |
                QDockWidget::DockWidgetFloatable);
            result->setWidget(content);
            addDockWidget(area, result);
            viewMenu->addAction(result->toggleViewAction());
            return result;
        };
        auto* hierarchy = new QTreeWidget(this);
        hierarchy->setHeaderLabel("Editor workspace");
        auto* root = new QTreeWidgetItem(hierarchy, {"Host document draft"});
        root->addChild(new QTreeWidgetItem({"Scene editing not connected yet"}));
        hierarchy->expandAll();
        dock("Hierarchy", "HierarchyDock", hierarchy, Qt::LeftDockWidgetArea);
        m_DraftTabs = new QTabWidget(this);
        m_DraftTabs->setObjectName("DraftTabs");
        m_DraftTabs->setTabsClosable(true);
        m_DraftTabs->setMovable(true);
        m_DraftTabs->setUsesScrollButtons(true);
        m_DraftTabs->tabBar()->hide();
        auto* documents = new QWidget(this);
        auto* documentLayout = new QVBoxLayout(documents);
        documentLayout->setContentsMargins(0, 0, 0, 0); documentLayout->setSpacing(0);
        documentLayout->addWidget(strip);
        documentLayout->addWidget(m_DraftTabs);
        dock("Documents", "DocumentDock", documents, Qt::RightDockWidgetArea);
        connect(m_DraftTabs, &QTabWidget::currentChanged, this, [this](int) { RefreshDocument(); });
        connect(m_DraftTabs, &QTabWidget::tabCloseRequested, this, [this](int index) { CloseDraft(index); });
        connect(m_DraftTabs->tabBar(), &QTabBar::tabMoved, this, [this](int, int) { RefreshDocument(); });
        m_Console = new ConsoleOutput(this);
        dock("Console", "ConsoleDock", m_Console, Qt::BottomDockWidgetArea);
        m_Viewport = new ViewportWidget(this);
        m_Viewport->OnSize = [this](QSize size) { m_Worker.SetViewportSize(size); };
        dock("Viewport", "ViewportDock", m_Viewport, Qt::LeftDockWidgetArea);
        m_Inspector = new QPlainTextEdit(this);
        m_Inspector->setObjectName("ResourceInspector");
        m_Inspector->setReadOnly(true);
        dock("Inspector", "InspectorDock", m_Inspector, Qt::RightDockWidgetArea);
        m_Workspace = new ProjectWorkspace(this);
        m_Workspace->OnInspector = [this](const QString& text)
        { if (m_Inspector->toPlainText() != text) m_Inspector->setPlainText(text); };
        m_Workspace->OnLog = [this](const QString& text)
        {
            if (m_Workspace->Root().isEmpty())
            {
                if (!m_Project.Path.isEmpty()) ClearProjectScene("The project folder is unavailable.");
                m_Project = {}; m_PackageOutputDirectory.clear();
            }
            m_CloseProject->setEnabled(!m_Workspace->Root().isEmpty());
            RefreshPackagingActions(); Log(text); RefreshDocument();
        };
        dock("Assets", "AssetsDock", m_Workspace, Qt::LeftDockWidgetArea);
        m_StateLabel = new QLabel(this);
        statusBar()->addWidget(m_StateLabel, 1);
        auto* file = menus->addMenu("File");
        auto* openProject = file->addAction("Open project folder...");
        openProject->setObjectName("OpenProjectAction");
        connect(openProject, &QAction::triggered, this, [this]
        {
            const auto directory = QFileDialog::getExistingDirectory(this, "Open project folder", m_Workspace->Root());
            if (!directory.isEmpty() && !OpenProject(directory))
                QMessageBox::warning(this, "Project open failed", m_Error);
        });
        m_CloseProject = file->addAction("Close project");
        m_CloseProject->setEnabled(false);
        connect(m_CloseProject, &QAction::triggered, this, [this] { CloseProject(); });
        InitProjectActions(file, menus);
        auto* openScene = file->addAction("Open scene for rendering...");
        openScene->setObjectName("OpenRenderSceneAction");
        connect(openScene, &QAction::triggered, this, [this]
        {
            const auto path = QFileDialog::getOpenFileName(this, "Open engine scene", m_Workspace->Root(), "Engine scene (*.GEScene)");
            if (!path.isEmpty() && !OpenScene(path)) QMessageBox::warning(this, "Scene open failed", m_Error);
        });
        auto* sampleScene = file->addAction("Show engine sample scene");
        connect(sampleScene, &QAction::triggered, this, [this] { OpenScene({}); });
        file->addSeparator();
        auto* fresh = file->addAction("New draft");
        fresh->setShortcut(QKeySequence::New);
        fresh->setShortcuts({QKeySequence::New, QKeySequence(Qt::CTRL | Qt::Key_T)});
        connect(fresh, &QAction::triggered, this, [this] { NewDraft(); });
        auto* open = file->addAction("Open draft...");
        open->setShortcut(QKeySequence::Open);
        connect(open, &QAction::triggered, this, [this]
        {
            const auto path = QFileDialog::getOpenFileName(this, "Open editor draft", {}, "Editor draft (*.gedraft)");
            if (!path.isEmpty() && !OpenDraft(path)) QMessageBox::warning(this, "Open failed", m_Error);
        });
        auto* save = file->addAction("Save draft");
        save->setShortcut(QKeySequence::Save);
        connect(save, &QAction::triggered, this, [this]
        {
            if (ActiveDraft().Path.isEmpty()) SaveAs();
            else if (!SaveDraft(ActiveDraft().Path)) QMessageBox::warning(this, "Save failed", m_Error);
        });
        auto* saveAs = file->addAction("Save draft as...");
        saveAs->setShortcut(QKeySequence::SaveAs);
        connect(saveAs, &QAction::triggered, this, [this] { SaveAs(); });
        auto* closeDraft = file->addAction("Close document tab");
        closeDraft->setShortcut(QKeySequence::Close);
        connect(closeDraft, &QAction::triggered, this, [this] { CloseDraft(m_DraftTabs->currentIndex()); });
        connect(file->addAction("Exit"), &QAction::triggered, this, &QWidget::close);
        auto* edit = menus->addMenu("Edit");
        m_Undo = edit->addAction("Undo"); m_Undo->setShortcut(QKeySequence::Undo);
        m_Redo = edit->addAction("Redo"); m_Redo->setShortcut(QKeySequence::Redo);
        m_Undo->setObjectName("UndoAction"); m_Redo->setObjectName("RedoAction");
        connect(m_Undo, &QAction::triggered, this, [this] { Document().Undo(); RefreshDocument(); });
        connect(m_Redo, &QAction::triggered, this, [this] { Document().Redo(); RefreshDocument(); });
        auto* workerMenu = menus->addMenu("Engine");
        menus->removeAction(viewMenu->menuAction());
        menus->insertMenu(workerMenu->menuAction(), viewMenu);
        auto* toolbar = addToolBar("Worker"); toolbar->setObjectName("WorkerToolbar");
        toolbar->setMovable(true);
        toolbar->setFloatable(true);
        viewMenu->addSeparator();
        viewMenu->addAction(toolbar->toggleViewAction());
        auto* resetLayout = viewMenu->addAction("Reset layout");
        resetLayout->setObjectName("ResetLayoutAction");
        connect(resetLayout, &QAction::triggered, this, [this] { ResetLayout(); });
        m_Start = workerMenu->addAction("Start / restart worker");
        m_Stop = workerMenu->addAction("Stop worker");
        toolbar->addAction(m_Start); toolbar->addAction(m_Stop);
        connect(m_Start, &QAction::triggered, this, [this] { StartWorker(); });
        connect(m_Stop, &QAction::triggered, this, [this] { m_Worker.Stop(); });
        m_Crash = workerMenu->addAction("Inject child-process native crash");
        m_Hang = workerMenu->addAction("Inject child-process hang");
        m_Malformed = workerMenu->addAction("Inject malformed pipe data");
        connect(m_Crash, &QAction::triggered, this, [this] { m_Worker.Inject(Protocol::Kind::Crash); });
        connect(m_Hang, &QAction::triggered, this, [this] { m_Worker.Inject(Protocol::Kind::Hang); });
        connect(m_Malformed, &QAction::triggered, this, [this] { m_Worker.Inject(Protocol::Kind::Malformed); });
        NewDraft();
        m_Worker.OnLog = [this](const QString& text) { Log(text); };
        m_Worker.OnConsoleOutput = [this](const QByteArray& bytes)
        {
            m_Console->AppendBytes(bytes);
            if (QCoreApplication::arguments().contains("--self-test")) std::cerr << bytes.toStdString();
        };
        m_Worker.OnConsoleReset = [this] { m_Console->ResetStream(); };
        m_Worker.OnState = [this](WorkerSupervisor::State)
        {
            RefreshWorker();
            if (m_RestartForScene && !m_Worker.Running())
            {
                m_RestartForScene = false;
                QTimer::singleShot(0, this, [this] { if (!m_ClosePending) StartWorker(); });
            }
            if (m_ClosePending && !m_Worker.Running()) QTimer::singleShot(0, this, [this] { close(); });
        };
        m_Worker.OnFrame = [this](const QImage& image)
        { if (!m_ProjectViewportEmpty) { m_HasImage = true; ++m_FrameCount; m_Viewport->Engine = m_Worker.IsEngine(); m_Viewport->Image = image; m_Viewport->update(); } };
        bool restored = false;
        if (!QCoreApplication::arguments().contains("--self-test"))
        {
            QSettings settings;
            restoreGeometry(settings.value("hostGeometry").toByteArray());
            restored = restoreState(settings.value("hostLayout").toByteArray(), 4);
        }
        if (!restored) QTimer::singleShot(0, this, [this] { ResetLayout(); });
        else QTimer::singleShot(0, this, [this] { RefreshDockTabs(); });
        Log("Qt Host: Engine menu starts the render worker. Scene editing/Play/Gizmo are not implemented.");
        m_Workspace->Close();
        RefreshDocument(); RefreshWorker();
    }
    MainWindow::~MainWindow()
    {
        if (m_PackagingProcess)
        {
            disconnect(m_PackagingProcess, nullptr, this, nullptr);
            if (PackagingRunning()) { m_PackagingProcess->kill(); m_PackagingProcess->waitForFinished(3000); }
        }
        qApp->removeEventFilter(this);
        for (auto* panel : findChildren<QDockWidget*>()) disconnect(panel, nullptr, this, nullptr);
        disconnect(m_DraftTabs, nullptr, this, nullptr);
        disconnect(m_DraftTabs->tabBar(), nullptr, this, nullptr);
        disconnect(m_BrowserTabs, nullptr, this, nullptr);
        for (const auto& entry : m_Drafts) disconnect(entry.first, nullptr, this, nullptr);
        m_Workspace->OnInspector = {}; m_Workspace->OnLog = {};
        m_Worker.OnState = {}; m_Worker.OnLog = {}; m_Worker.OnFrame = {};
        m_Worker.OnConsoleOutput = {}; m_Worker.OnConsoleReset = {};
    }
    void MainWindow::ResetLayout()
    {
        auto* hierarchy = findChild<QDockWidget*>("HierarchyDock");
        auto* document = findChild<QDockWidget*>("DocumentDock");
        auto* console = findChild<QDockWidget*>("ConsoleDock");
        auto* viewport = findChild<QDockWidget*>("ViewportDock");
        auto* assets = findChild<QDockWidget*>("AssetsDock");
        auto* inspector = findChild<QDockWidget*>("InspectorDock");
        for (auto* panel : {hierarchy, document, console, viewport, assets, inspector})
        {
            panel->setFloating(false);
            removeDockWidget(panel);
        }
        addDockWidget(Qt::LeftDockWidgetArea, hierarchy);
        addDockWidget(Qt::LeftDockWidgetArea, viewport);
        splitDockWidget(hierarchy, viewport, Qt::Horizontal);
        addDockWidget(Qt::LeftDockWidgetArea, document);
        splitDockWidget(viewport, document, Qt::Horizontal);
        addDockWidget(Qt::LeftDockWidgetArea, console);
        splitDockWidget(viewport, console, Qt::Vertical);
        addDockWidget(Qt::LeftDockWidgetArea, assets);
        splitDockWidget(hierarchy, assets, Qt::Vertical);
        addDockWidget(Qt::LeftDockWidgetArea, inspector);
        tabifyDockWidget(document, inspector);
        for (auto* panel : {hierarchy, document, console, viewport, assets, inspector}) panel->show();
        inspector->raise();
        resizeDocks({hierarchy, viewport, document}, {220, 740, 300}, Qt::Horizontal);
        resizeDocks({viewport, console}, {580, 180}, Qt::Vertical);
        resizeDocks({hierarchy, assets}, {300, 460}, Qt::Vertical);
        RefreshDockTabs();
        auto* toolbar = findChild<QToolBar*>("WorkerToolbar");
        addToolBar(Qt::TopToolBarArea, toolbar);
        toolbar->show();
    }
    bool MainWindow::eventFilter(QObject* watched, QEvent* event)
    {
        if (!m_DockTabsUpdating && !m_DockTabsUpdatePending &&
            (watched == this || watched->objectName() == "BrowserDockTabs") &&
            (event->type() == QEvent::LayoutRequest || event->type() == QEvent::Show ||
                event->type() == QEvent::Resize))
        {
            m_DockTabsUpdatePending = true;
            QTimer::singleShot(0, this, [this]
            { m_DockTabsUpdatePending = false; RefreshDockTabs(); });
        }
        return QMainWindow::eventFilter(watched, event);
    }
    QDockWidget* MainWindow::DockForTab(QTabBar* bar, int index) const
    {
        if (!bar || bar == m_BrowserTabs || (m_DraftTabs && m_DraftTabs->isAncestorOf(bar)) ||
            index < 0 || index >= bar->count()) return nullptr;
        // The six tool panels have unique fixed titles. Leave Qt's private tabData untouched.
        for (auto* panel : findChildren<QDockWidget*>())
            if (panel->windowTitle() == bar->tabText(index)) return panel;
        return nullptr;
    }
    void MainWindow::RefreshDockTabs()
    {
        if (m_DockTabsUpdating) return;
        QScopedValueRollback<bool> updating(m_DockTabsUpdating, true);
        for (auto* widget : QApplication::allWidgets())
        {
            auto* bar = qobject_cast<QTabBar*>(widget);
            if (!DockForTab(bar, 0)) continue;
            if (bar->objectName() != "BrowserDockTabs")
            {
                bar->setObjectName("BrowserDockTabs");
                bar->setDocumentMode(true); bar->setDrawBase(false);
                bar->setExpanding(false); bar->setTabsClosable(false);
                bar->setUsesScrollButtons(true); bar->setElideMode(Qt::ElideRight);
                bar->setStyleSheet(
                    "QTabBar { background: #303238; } "
                    "QTabBar::tab { min-width: 80px; max-width: 220px; padding: 3px 12px; "
                    "border: none; border-top-left-radius: 6px; border-top-right-radius: 6px; "
                    "background: #53565e; color: #e8eaed; margin-right: 2px; } "
                    "QTabBar::tab:selected { background: #202226; color: white; } "
                    "QTabBar::tab:hover:!selected { background: #626670; }");
            }
            for (int i = 0; i < bar->count(); ++i)
            {
                auto* panel = DockForTab(bar, i);
                if (!panel) continue;
                const bool last = i == bar->count() - 1;
                auto* old = bar->tabButton(i, QTabBar::RightSide);
                if (old && old->property("DockName").toString() == panel->objectName() &&
                    old->property("HasAdd").toBool() == last) continue;
                if (old) { bar->setTabButton(i, QTabBar::RightSide, nullptr); old->deleteLater(); }
                auto* controls = new QWidget(bar);
                controls->setProperty("DockName", panel->objectName());
                controls->setProperty("HasAdd", last);
                auto* layout = new QHBoxLayout(controls);
                layout->setContentsMargins(0, 0, 0, 0); layout->setSpacing(6);
                auto button = [controls, layout](const QString& text, const QString& tip)
                {
                    auto* result = new QToolButton(controls);
                    result->setText(text); result->setToolTip(tip); result->setFixedSize(20, 20);
                    result->setStyleSheet("QToolButton { color: #e8eaed; background: transparent; "
                        "border: none; font-size: 16px; } QToolButton:hover { background: #747780; border-radius: 6px; } "
                        "QToolButton::menu-indicator { image: none; }");
                    layout->addWidget(result); return result;
                };
                auto* close = button(QString::fromUtf8("\xc3\x97"), "Hide " + panel->windowTitle());
                close->setObjectName("CloseDockTabButton");
                QPointer<QDockWidget> guardedPanel(panel);
                connect(close, &QToolButton::clicked, this, [guardedPanel]
                { if (guardedPanel) guardedPanel->hide(); });
                if (last)
                {
                    auto* add = button("+", "Add panel to this dock group");
                    add->setObjectName("AddDockTabButton");
                    add->setPopupMode(QToolButton::InstantPopup);
                    auto* menu = new QMenu(add); add->setMenu(menu);
                    QPointer<QTabBar> guardedBar(bar);
                    connect(menu, &QMenu::aboutToShow, this, [this, menu, guardedBar]
                    {
                        menu->clear();
                        for (auto* candidate : findChildren<QDockWidget*>())
                        {
                            auto* action = menu->addAction(candidate->windowTitle());
                            QPointer<QDockWidget> target(candidate);
                            connect(action, &QAction::triggered, this, [this, guardedBar, target]
                            {
                                auto* anchor = guardedBar ? DockForTab(guardedBar, guardedBar->currentIndex()) : nullptr;
                                if (!anchor || !target) return;
                                if (anchor != target) { target->setFloating(false); tabifyDockWidget(anchor, target); }
                                target->show(); target->raise();
                            });
                        }
                    });
                }
                controls->setFixedSize(last ? 46 : 20, 20);
                bar->setTabButton(i, QTabBar::RightSide, controls);
            }
        }
    }
    void MainWindow::Log(const QString& text)
    {
        const auto bounded = text.left(16 * 1024).trimmed();
        m_Console->AppendMessage(bounded);
        if (QCoreApplication::arguments().contains("--self-test"))
            std::cerr << bounded.toStdString() << '\n';
    }
    MainWindow::DraftTab& MainWindow::ActiveDraft()
    {
        return *m_Drafts.at(m_DraftTabs->currentWidget());
    }
    DraftDocument& MainWindow::Document() { return ActiveDraft().Document; }
    int MainWindow::DraftCount() const { return m_DraftTabs->count(); }
    bool MainWindow::NewDraft()
    {
        if (DraftCount() >= 32) { m_Error = "At most 32 document tabs can be open."; Log(m_Error); return false; }
        auto tab = std::make_unique<DraftTab>();
        tab->Untitled = ++m_NextUntitled;
        auto* editor = new QPlainTextEdit(m_DraftTabs);
        tab->Editor = editor;
        editor->setUndoRedoEnabled(false);
        editor->setPlaceholderText("Draft notes; .GEScene editing is not connected yet.");
        editor->setPlainText(QString::fromStdString(tab->Document.Text()));
        m_Drafts.emplace(editor, std::move(tab));
        connect(editor, &QPlainTextEdit::textChanged, this, [this, editor]
        {
            auto& draft = *m_Drafts.at(editor);
            const auto text = editor->toPlainText().toUtf8();
            if (text.size() > 64 * 1024) { Log("Draft size limit is 64 KiB UTF-8."); RefreshDocument(); return; }
            draft.Document.Edit(text.toStdString()); RefreshDocument();
        });
        {
            QSignalBlocker blocker(m_DraftTabs);
            m_DraftTabs->setCurrentIndex(m_DraftTabs->addTab(editor, "Untitled"));
        }
        m_Error.clear(); RefreshDocument();
        findChild<QDockWidget*>("DocumentDock")->show();
        findChild<QDockWidget*>("DocumentDock")->raise();
        editor->setFocus();
        return true;
    }
    bool MainWindow::CloseDraft(int index)
    {
        if (index < 0 || index >= DraftCount() || m_ClosePending || m_ConfirmingClose) return false;
        QScopedValueRollback<bool> confirming(m_ConfirmingClose, true);
        auto* previous = m_DraftTabs->currentWidget();
        auto* page = m_DraftTabs->widget(index);
        m_DraftTabs->setCurrentIndex(index);
        if (!ConfirmDiscard()) { m_DraftTabs->setCurrentWidget(previous); return false; }
        {
            QSignalBlocker blocker(m_DraftTabs);
            m_DraftTabs->removeTab(index);
            disconnect(page, nullptr, this, nullptr);
            m_Drafts.erase(page);
            delete page;
        }
        if (!DraftCount()) NewDraft();
        else
        {
            if (previous != page) m_DraftTabs->setCurrentWidget(previous);
            RefreshDocument();
        }
        return true;
    }
    void MainWindow::SetDraftText(const QString& text) { ActiveDraft().Editor->setPlainText(text); }
    bool MainWindow::OpenProject(const QString& path)
    {
        if (PackagingRunning()) { m_Error = "Wait for packaging to finish or cancel it before switching projects."; return false; }
        ProjectFile project;
        const bool directory = QFileInfo(path).isDir();
        if (!directory && !ProjectFile::Load(path, project, m_Error)) { Log(m_Error); return false; }
        if (!m_Workspace->Open(directory ? path : project.Root()))
        { m_Error = "Project folder is missing or unreadable."; Log(m_Error); return false; }
        m_Project = project;
        m_PackageOutputDirectory.clear();
        m_PackageExecutableName.clear();
        m_PackageLogoPath.clear();
        delete m_PackageIconFile;
        m_PackageIconFile = nullptr;
        if (!m_Project.Path.isEmpty())
        {
            const auto key = QCryptographicHash::hash(m_Project.Path.toCaseFolded().toUtf8(), QCryptographicHash::Sha256).toHex();
            QSettings settings;
            m_PackageOutputDirectory = settings.value("packagingOutputs/" + QString::fromLatin1(key)).toString();
            m_PackageExecutableName = settings.value("packagingExecutableNames/" + QString::fromLatin1(key)).toString();
            m_PackageLogoPath = settings.value("packagingLogos/" + QString::fromLatin1(key)).toString();
            if (m_PackageOutputDirectory.isEmpty())
                m_PackageOutputDirectory = QDir(m_Project.Root()).absoluteFilePath("../" + m_Project.Name + "-Package");
            if (m_PackageExecutableName.isEmpty()) m_PackageExecutableName = m_Project.Name;
        }
        m_CloseProject->setEnabled(true);
        if (!directory) ApplyProjectScene();
        m_Error.clear(); RefreshDocument(); RefreshPackagingActions();
        Log(directory ? "Opened project folder: " + m_Workspace->Root() : "Opened project: " + m_Project.Path);
        return true;
    }
    void MainWindow::CloseProject()
    {
        if (PackagingRunning()) { Log("Cancel packaging before closing the project."); return; }
        const bool hadProject = !m_Project.Path.isEmpty();
        m_Project = {};
        if (hadProject) ClearProjectScene("No project scene is open.");
        m_PackageOutputDirectory.clear();
        m_PackageExecutableName.clear();
        m_PackageLogoPath.clear();
        delete m_PackageIconFile;
        m_PackageIconFile = nullptr;
        m_Workspace->Close(); m_CloseProject->setEnabled(false);
        RefreshDocument(); RefreshPackagingActions(); Log("Project closed; drafts are preserved.");
    }
    void MainWindow::ClearProjectScene(const QString& reason)
    {
        m_ProjectViewportEmpty = true; m_RestartForScene = false; m_ScenePath.clear();
        m_HasImage = false; m_FrameCount = 0; m_Viewport->Image = {};
        m_Viewport->EmptyMessage = reason; m_Viewport->update();
        m_Worker.Stop(); RefreshWorker();
    }
    void MainWindow::ApplyProjectScene()
    {
        QString error;
        const auto scene = m_Project.ResolveDefaultScene(error);
        if (scene.isEmpty())
        { ClearProjectScene(error); Log(error); return; }
        if (!OpenScene(scene)) { const auto reason = m_Error; ClearProjectScene(reason); Log(reason); }
    }
    QString MainWindow::InspectorText() const { return m_Inspector->toPlainText(); }
    void MainWindow::RefreshDocument()
    {
        if (!DraftCount() || !m_Undo || !m_Redo || !m_Workspace) return;
        bool anyDirty = false;
        for (const auto& entry : m_Drafts)
        {
            auto& draft = *entry.second;
            const auto text = QString::fromStdString(draft.Document.Text());
            if (text != draft.Editor->toPlainText())
            { QSignalBlocker blocker(draft.Editor); draft.Editor->setPlainText(text); }
            const auto index = m_DraftTabs->indexOf(entry.first);
            const auto title = draft.Path.isEmpty() ? QString("Untitled %1").arg(draft.Untitled) : QFileInfo(draft.Path).fileName();
            m_DraftTabs->setTabText(index, title + (draft.Document.Dirty() ? " *" : ""));
            m_DraftTabs->setTabToolTip(index, draft.Path.isEmpty() ? "Unsaved draft" : draft.Path);
            anyDirty |= draft.Document.Dirty();
        }
        m_Undo->setEnabled(Document().CanUndo()); m_Redo->setEnabled(Document().CanRedo());
        {
            QSignalBlocker blocker(m_BrowserTabs);
            while (m_BrowserTabs->count() > DraftCount()) m_BrowserTabs->removeTab(m_BrowserTabs->count() - 1);
            for (int index = 0; index < DraftCount(); ++index)
            {
                if (index == m_BrowserTabs->count()) m_BrowserTabs->addTab(m_DraftTabs->tabText(index));
                else m_BrowserTabs->setTabText(index, m_DraftTabs->tabText(index));
                m_BrowserTabs->setTabToolTip(index, m_DraftTabs->tabToolTip(index));
            }
            m_BrowserTabs->setCurrentIndex(m_DraftTabs->currentIndex());
        }
        const auto project = !m_Project.Name.isEmpty() ? m_Project.Name :
            m_Workspace->Root().isEmpty() ? "No project" : QFileInfo(m_Workspace->Root()).fileName();
        setWindowTitle(QString("GEngine Qt Editor - %1 - %2%3").arg(project,
            m_DraftTabs->tabText(m_DraftTabs->currentIndex()).remove(" *"), anyDirty ? " *" : ""));
    }
    void MainWindow::RefreshWorker()
    {
        const auto description = m_Worker.Description();
        m_StateLabel->setText(description);
        m_Viewport->State = description; m_Viewport->update();
        m_Start->setEnabled(!m_ProjectViewportEmpty && !m_Worker.Running() && m_Worker.GetState() != WorkerSupervisor::State::Starting);
        m_Stop->setEnabled(m_Worker.Running() || m_Worker.GetState() == WorkerSupervisor::State::Starting);
        const bool ready = m_Worker.GetState() == WorkerSupervisor::State::Ready;
        m_Crash->setEnabled(ready); m_Hang->setEnabled(ready); m_Malformed->setEnabled(ready);
    }
    void MainWindow::StartWorker()
    {
        if (m_ProjectViewportEmpty || m_Worker.Running()) return;
        m_HasImage = false; m_FrameCount = 0; m_Viewport->Image = {};
        m_Worker.SetViewportSize(m_Viewport->isVisible() ? QSize(qRound(m_Viewport->width() * m_Viewport->devicePixelRatioF()),
            qRound(m_Viewport->height() * m_Viewport->devicePixelRatioF())) : QSize{});
        m_Worker.Start(m_WorkerPath, m_ScenePath);
    }
    bool MainWindow::OpenScene(const QString& path)
    {
        const QFileInfo file(path);
        if (!path.isEmpty() && (!file.isFile() || file.suffix().compare("GEScene", Qt::CaseInsensitive) != 0))
        { m_Error = "Select an existing .GEScene file."; return false; }
        m_Error.clear(); m_ScenePath = path.isEmpty() ? QString{} : file.absoluteFilePath();
        m_ProjectViewportEmpty = false; m_Viewport->EmptyMessage.clear();
        m_Viewport->Scene = path.isEmpty() ? "Engine sample scene" : file.fileName();
        m_Viewport->Image = {}; m_Viewport->update();
        if (m_Worker.Running()) { m_RestartForScene = true; m_Worker.Stop(); }
        else StartWorker();
        return true;
    }
    bool MainWindow::HasEngineEvidence() const
    {
        const auto& image = m_Viewport->Image;
        if (!m_Worker.IsEngine() || image.width() < 320 || image.height() < 200) return false;
        const auto left = image.pixelColor(image.width() / 4, image.height() / 2);
        bool cyan = false;
        for (int y = image.height() / 4; y < image.height() * 3 / 4; ++y)
        {
            const auto color = image.pixelColor(image.width() * 3 / 4, y);
            cyan |= color.blue() > 230 && color.green() > 150 && color.red() < 80;
        }
        const auto corner = image.pixelColor(2, 2);
        return left.red() > 220 && left.green() < 100 && cyan &&
            corner.blue() < 100 &&
            m_Console->toPlainText().contains("Editor engine frame 1 delivered");
    }
    bool MainWindow::SaveDraft(const QString& path)
    {
        auto key = [](const QString& value)
        {
            const QFileInfo info(value);
            auto normalized = info.canonicalFilePath();
            if (normalized.isEmpty()) normalized = QDir::cleanPath(info.absoluteFilePath());
            return normalized.toCaseFolded();
        };
        for (const auto& entry : m_Drafts)
            if (entry.first != m_DraftTabs->currentWidget() && !entry.second->Path.isEmpty() && key(entry.second->Path) == key(path))
            { m_Error = "This file is already open in another document tab."; return false; }
        const QJsonObject object{{"format", "GEngine.EditorDraft"}, {"version", 1},
            {"notes", QString::fromStdString(Document().Text())}};
        const auto bytes = QJsonDocument(object).toJson();
        QSaveFile file(path);
        file.setDirectWriteFallback(false);
        if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        { m_Error = file.errorString(); return false; }
        m_Error.clear(); ActiveDraft().Path = QFileInfo(path).canonicalFilePath(); Document().MarkSaved(); RefreshDocument();
        return true;
    }
    bool MainWindow::OpenDraft(const QString& path)
    {
        const QFileInfo requested(path);
        auto key = requested.canonicalFilePath();
        if (key.isEmpty()) key = QDir::cleanPath(requested.absoluteFilePath());
        for (const auto& entry : m_Drafts)
        {
            if (!entry.second->Path.isEmpty() && entry.second->Path.compare(key, Qt::CaseInsensitive) == 0)
            {
                m_DraftTabs->setCurrentWidget(entry.first); m_Error.clear(); RefreshDocument();
                findChild<QDockWidget*>("DocumentDock")->show(); findChild<QDockWidget*>("DocumentDock")->raise();
                return true;
            }
        }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) { m_Error = file.errorString(); return false; }
        if (file.size() > 1024 * 1024) { m_Error = "Draft file exceeds 1 MiB."; return false; }
        QJsonParseError error;
        const auto bytes = file.read(1024 * 1024 + 1);
        if (bytes.size() > 1024 * 1024) { m_Error = "Draft file exceeds 1 MiB."; return false; }
        const auto json = QJsonDocument::fromJson(bytes, &error);
        const auto object = json.object();
        if (error.error != QJsonParseError::NoError || object.value("format") != "GEngine.EditorDraft" ||
            object.value("version").toDouble() != 1 || !object.value("notes").isString() ||
            object.size() != 3 || object.value("notes").toString().toUtf8().size() > 64 * 1024)
        { m_Error = "Unsupported editor draft; .GEScene is not supported by this prototype."; return false; }
        if (!NewDraft()) return false;
        m_Error.clear(); ActiveDraft().Path = key;
        Document().Reset(object.value("notes").toString().toUtf8().toStdString());
        RefreshDocument();
        return true;
    }
    bool MainWindow::SaveAs()
    {
        auto path = QFileDialog::getSaveFileName(this, "Save editor draft", ActiveDraft().Path, "Editor draft (*.gedraft)");
        if (path.isEmpty()) return false;
        if (!path.endsWith(".gedraft", Qt::CaseInsensitive)) path += ".gedraft";
        if (SaveDraft(path)) return true;
        QMessageBox::warning(this, "Save failed", m_Error);
        return false;
    }
    bool MainWindow::ConfirmDiscard()
    {
        if (!Document().Dirty()) return true;
        const auto answer = QMessageBox::question(this, "Unsaved editor draft", "Save " +
            m_DraftTabs->tabText(m_DraftTabs->currentIndex()) + " before closing?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (answer == QMessageBox::Cancel) return false;
        if (answer == QMessageBox::Discard) return true;
        if (ActiveDraft().Path.isEmpty()) return SaveAs();
        if (SaveDraft(ActiveDraft().Path)) return true;
        QMessageBox::warning(this, "Save failed", m_Error);
        return false;
    }
    void MainWindow::closeEvent(QCloseEvent* event)
    {
        if (m_ConfirmingClose) { event->ignore(); return; }
        if (!m_ClosePending && !ConfirmAllDrafts()) { event->ignore(); return; }
        m_ClosePending = true;
        setEnabled(false);
        if (PackagingRunning()) { event->ignore(); CancelPackaging(); return; }
        if (m_Worker.Running()) { event->ignore(); m_Worker.Stop(); return; }
        if (!QCoreApplication::arguments().contains("--self-test"))
        {
            QSettings settings;
            settings.setValue("hostGeometry", saveGeometry());
            settings.setValue("hostLayout", saveState(4));
        }
        event->accept();
    }
    bool MainWindow::ConfirmAllDrafts()
    {
        QScopedValueRollback<bool> confirming(m_ConfirmingClose, true);
        auto* previous = m_DraftTabs->currentWidget();
        for (int index = 0; index < DraftCount(); ++index)
        {
            m_DraftTabs->setCurrentIndex(index);
            if (!ConfirmDiscard()) { m_DraftTabs->setCurrentWidget(previous); return false; }
        }
        m_DraftTabs->setCurrentWidget(previous);
        return true;
    }
}
