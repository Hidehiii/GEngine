#include "MainWindow.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QTemporaryDir>
#include <QTimer>
#include <QDockWidget>
#include <QAction>
#include <QDir>
#include <QTreeView>
#include <QFileSystemModel>
#include <QFileInfo>
#include <QTabWidget>
#include <QTabBar>
#include <QMessageBox>
#include <QAbstractButton>
#include <QToolButton>
#include <QMenu>
#include <iostream>
#include <memory>

namespace GEngineEditor
{
    void RunRecoveryCheck(MainWindow& window)
    {
        struct Check
        {
            QTemporaryDir Directory;
            QElapsedTimer Clock;
            int Phase = 0;
            int UiTicks = 0;
            int HangTick = 0;
            bool Done = false;
            QWidget* BackgroundTab = nullptr;
        };
        auto check = std::make_shared<Check>();
        check->Clock.start();
        auto* timer = new QTimer(&window);
        timer->setInterval(20);
        auto finish = [&window, check, timer](bool ok, const char* message)
        {
            if (check->Done) return;
            check->Done = true;
            timer->stop();
            std::cout << (ok ? "PASS: " : "FAIL: ") << message << std::endl;
            // Destruction owns a final kill fallback if a failing test leaves its child alive.
            QApplication::exit(ok ? 0 : 1);
        };
        auto answerDialogs = [&window](std::vector<QMessageBox::StandardButton> answers)
        {
            auto remaining = std::make_shared<std::vector<QMessageBox::StandardButton>>(std::move(answers));
            auto attempts = std::make_shared<int>(0);
            auto* responder = new QTimer(&window);
            responder->setInterval(10);
            QObject::connect(responder, &QTimer::timeout, responder, [remaining, attempts, responder]
            {
                auto* box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
                if (++*attempts > 500)
                {
                    std::cerr << "FAIL: expected document confirmation dialog was not handled\n";
                    responder->stop(); if (box) box->reject(); QApplication::exit(1); return;
                }
                if (!box || remaining->empty()) return;
                const auto answer = remaining->front();
                if (auto* button = box->button(answer))
                { remaining->erase(remaining->begin()); button->click(); }
                else
                {
                    std::cerr << "FAIL: unexpected dialog: " << box->windowTitle().toStdString()
                        << ": " << box->text().toStdString() << '\n';
                    remaining->clear(); box->reject(); QApplication::exit(1);
                }
                if (remaining->empty()) { responder->stop(); responder->deleteLater(); }
            });
            responder->start();
        };
        QObject::connect(timer, &QTimer::timeout, &window, [&window, check, timer, finish, answerDialogs]
        {
            using State = WorkerSupervisor::State;
            if (++check->UiTicks > 6000 || check->Clock.elapsed() > 120000)
            { finish(false, "recovery check deadline exceeded"); return; }
            auto& worker = window.Worker();
            auto& document = window.Document();
            const auto state = worker.GetState();
            if ((check->Phase == 2 || check->Phase == 4 || check->Phase == 7 || check->Phase == 9) &&
                state == State::Disconnected && !worker.Running())
            { finish(false, "worker disconnected before producing a frame"); return; }
            if ((check->Phase == 2 || check->Phase == 4 || check->Phase == 7 || check->Phase == 9) &&
                state == State::Disconnected && !worker.Running())
            { finish(false, "worker disconnected before producing a frame"); return; }
            switch (check->Phase)
            {
            case 0:
                timer->stop();
                for (auto area : {Qt::LeftDockWidgetArea, Qt::RightDockWidgetArea,
                    Qt::TopDockWidgetArea, Qt::BottomDockWidgetArea})
                    if (window.tabPosition(area) != QTabWidget::North)
                    { finish(false, "dock panel tabs must appear above their content"); return; }
                if (!check->Directory.isValid()) { finish(false, "temporary directory unavailable"); return; }
                {
                    auto* tabs = window.findChild<QTabWidget*>("DraftTabs");
                    auto* browserTabs = window.findChild<QTabBar*>("BrowserDocumentTabs");
                    auto* newTab = window.findChild<QToolButton*>("NewDocumentTabButton");
                    if (!tabs || !tabs->isMovable() || !tabs->tabsClosable())
                    { finish(false, "document tabs unavailable"); return; }
                    if (!browserTabs || !newTab || !window.findChild<QDockWidget*>("DocumentDock")->isAncestorOf(browserTabs) ||
                        tabs->isAncestorOf(browserTabs) || !tabs->tabBar()->isHidden() ||
                        browserTabs->count() != window.DraftCount())
                    { finish(false, "document-local tab strip unavailable"); return; }
                    if (window.findChild<QTabBar*>("BrowserPanelTabs"))
                    { finish(false, "duplicate global panel navigation still exists"); return; }
                    const auto dockLayout = window.saveState(4);
                    auto* viewportPanel = window.findChild<QDockWidget*>("ViewportDock");
                    auto* hierarchyPanel = window.findChild<QDockWidget*>("HierarchyDock");
                    auto* consolePanel = window.findChild<QDockWidget*>("ConsoleDock");
                    window.tabifyDockWidget(viewportPanel, hierarchyPanel);
                    window.tabifyDockWidget(viewportPanel, consolePanel);
                    auto settleTabs = []
                    { for (int i = 0; i < 3; ++i) QApplication::processEvents(QEventLoop::ExcludeUserInputEvents); };
                    settleTabs();
                    QTabBar* dockTabs = nullptr;
                    for (auto* bar : window.findChildren<QTabBar*>("BrowserDockTabs"))
                        for (int i = 0; i < bar->count(); ++i)
                            if (bar->tabText(i) == "Viewport") dockTabs = bar;
                    if (!dockTabs || dockTabs->count() != 3 || !dockTabs->isMovable())
                    { finish(false, "browser-style native dock group unavailable"); return; }
                    dockTabs->moveTab(0, 2); settleTabs();
                    QToolButton* closeHierarchy = nullptr;
                    for (int i = 0; i < dockTabs->count(); ++i)
                    {
                        auto* controls = dockTabs->tabButton(i, QTabBar::RightSide);
                        if (!controls) { finish(false, "dock controls missing after reorder"); return; }
                        if (dockTabs->tabText(i) == "Hierarchy")
                            closeHierarchy = controls->findChild<QToolButton*>("CloseDockTabButton");
                    }
                    if (!closeHierarchy) { finish(false, "dock close button missing"); return; }
                    closeHierarchy->click(); settleTabs();
                    if (!hierarchyPanel->isHidden() || viewportPanel->isHidden() || consolePanel->isHidden())
                    { finish(false, "reordered dock close targeted wrong panel"); return; }
                    auto* addPanel = dockTabs->findChild<QToolButton*>("AddDockTabButton");
                    if (!addPanel || !addPanel->menu()) { finish(false, "dock add menu missing"); return; }
                    addPanel->menu()->aboutToShow();
                    for (auto* action : addPanel->menu()->actions())
                        if (action->text() == "Hierarchy") action->trigger();
                    settleTabs();
                    if (hierarchyPanel->isHidden() || !window.tabifiedDockWidgets(viewportPanel).contains(hierarchyPanel))
                    { finish(false, "dock menu did not reopen panel into its group"); return; }
                    if (!window.restoreState(dockLayout, 4)) { finish(false, "browser dock layout restore failed"); return; }
                    settleTabs();
                    auto* first = tabs->currentWidget();
                    window.SetDraftText("First document");
                    window.SetDraftText("First document changed");
                    window.Document().Undo();
                    newTab->click();
                    if (window.DraftCount() != 2 || browserTabs->count() != 2)
                    { finish(false, "new document button failed"); return; }
                    auto* second = tabs->currentWidget();
                    window.SetDraftText("Second saved");
                    const auto path = check->Directory.filePath("Console");
                    if (!window.SaveDraft(path)) { finish(false, "second document save failed"); return; }
                    settleTabs();
                    if (browserTabs->objectName() != "BrowserDocumentTabs" ||
                        tabs->tabBar()->objectName() == "BrowserDockTabs")
                    { finish(false, "document filename was confused with a dock panel"); return; }
                    window.SetDraftText("Second unsaved");
                    browserTabs->moveTab(1, 0);
                    browserTabs->setCurrentIndex(tabs->indexOf(first));
                    if (window.Document().Text() != "First document" || !window.Document().CanRedo() ||
                        window.SaveDraft(path) || !window.Document().Dirty())
                    { finish(false, "tab history isolation or save conflict failed"); return; }
                    QFile saved(path);
                    if (!saved.open(QIODevice::ReadOnly) || !saved.readAll().contains("Second saved"))
                    { finish(false, "save conflict overwrote another document"); return; }
                    saved.close();
                    const auto count = window.DraftCount();
                    if (!window.OpenDraft(path) || window.DraftCount() != count || tabs->currentWidget() != second ||
                        window.Document().Text() != "Second unsaved" || !window.Document().Dirty())
                    { finish(false, "duplicate open replaced unsaved document"); return; }
                    answerDialogs({QMessageBox::Cancel});
                    if (window.CloseDraft(tabs->indexOf(first)) || window.DraftCount() != count || tabs->currentWidget() != second)
                    { finish(false, "cancel closed document tab"); return; }
                    if (!window.NewDraft()) { finish(false, "temporary clean tab failed"); return; }
                    auto* clean = tabs->currentWidget(); tabs->setCurrentWidget(second);
                    if (!window.CloseDraft(tabs->indexOf(clean)) || tabs->currentWidget() != second || window.DraftCount() != count)
                    { finish(false, "background tab close changed active document"); return; }
                    answerDialogs({QMessageBox::Discard, QMessageBox::Cancel});
                    if (window.close() || !window.isEnabled() || window.DraftCount() != count)
                    { finish(false, "cancel exit removed document tabs"); return; }
                    if (tabs->currentWidget() != second || window.Document().Text() != "Second unsaved")
                    { finish(false, "cancel exit changed active document"); return; }
                    window.findChild<QAction*>("UndoAction")->trigger();
                    if (window.Document().Dirty() || window.Document().Text() != "Second saved")
                    { finish(false, "active tab undo did not restore its savepoint"); return; }
                    window.findChild<QAction*>("RedoAction")->trigger();
                    if (!window.Document().Dirty() || window.Document().Text() != "Second unsaved")
                    { finish(false, "active tab redo failed"); return; }
                    answerDialogs({QMessageBox::Save, QMessageBox::Cancel});
                    if (window.close() || window.DraftCount() != count || window.Document().Dirty())
                    { finish(false, "save before exit cancellation failed"); return; }
                    QFile savedOnExit(path);
                    if (!savedOnExit.open(QIODevice::ReadOnly) || !savedOnExit.readAll().contains("Second unsaved"))
                    { finish(false, "exit cancellation rolled back a completed save"); return; }
                    window.SetDraftText("Second unsaved again");
                    answerDialogs({QMessageBox::Discard});
                    if (!window.CloseDraft(tabs->currentIndex()))
                    { finish(false, "discard document close failed"); return; }
                    answerDialogs({QMessageBox::Discard});
                    if (!window.CloseDraft(tabs->currentIndex()) || window.DraftCount() != 1 ||
                        browserTabs->count() != 1 || window.Document().Dirty())
                    { finish(false, "last-tab replacement failed"); return; }
                }
                {
                    const auto originalLayout = window.saveState(2);
                    {
                        QMainWindow legacyLayout;
                        for (const auto* name : {"HierarchyDock", "DocumentDock", "ConsoleDock", "ViewportDock"})
                        {
                            auto* panel = new QDockWidget(name, &legacyLayout);
                            panel->setObjectName(name);
                            legacyLayout.addDockWidget(Qt::LeftDockWidgetArea, panel);
                        }
                        if (!window.restoreState(legacyLayout.saveState(2), 2) ||
                            !window.restoreState(originalLayout, 2))
                        { finish(false, "four-panel layout compatibility failed"); return; }
                    }
                    for (const auto* name : {"HierarchyDock", "DocumentDock", "ConsoleDock", "ViewportDock", "AssetsDock", "InspectorDock"})
                    {
                        auto* panel = window.findChild<QDockWidget*>(name);
                        if (!panel || panel->allowedAreas() != Qt::AllDockWidgetAreas ||
                            !panel->features().testFlag(QDockWidget::DockWidgetFloatable) ||
                            !panel->features().testFlag(QDockWidget::DockWidgetMovable) ||
                            !panel->features().testFlag(QDockWidget::DockWidgetClosable))
                        { finish(false, "panel docking capabilities missing"); return; }
                        panel->setFloating(true);
                        if (!panel->isFloating()) { finish(false, "panel did not float"); return; }
                        panel->hide(); panel->toggleViewAction()->trigger();
                        if (panel->isHidden()) { finish(false, "panel could not be reopened"); return; }
                        panel->setFloating(false);
                        window.addDockWidget(Qt::LeftDockWidgetArea, panel);
                        if (panel->isFloating()) { finish(false, "panel did not redock"); return; }
                    }
                    auto* hierarchy = window.findChild<QDockWidget*>("HierarchyDock");
                    auto* documentPanel = window.findChild<QDockWidget*>("DocumentDock");
                    auto* viewport = window.findChild<QDockWidget*>("ViewportDock");
                    auto* console = window.findChild<QDockWidget*>("ConsoleDock");
                    window.tabifyDockWidget(hierarchy, documentPanel);
                    if (!window.tabifiedDockWidgets(hierarchy).contains(documentPanel))
                    { finish(false, "panel tabbing failed"); return; }
                    viewport->setFloating(true); console->hide();
                    const auto changedLayout = window.saveState(2);
                    auto* reset = window.findChild<QAction*>("ResetLayoutAction");
                    if (!reset) { finish(false, "layout reset action missing"); return; }
                    reset->trigger();
                    if (!window.tabifiedDockWidgets(viewport).isEmpty() ||
                        !window.tabifiedDockWidgets(documentPanel).contains(window.findChild<QDockWidget*>("InspectorDock")))
                    { finish(false, "default split docking layout missing"); return; }
                    bool groupTabsAvailable = false;
                    for (auto* bar : window.findChildren<QTabBar*>(QString(), Qt::FindDirectChildrenOnly))
                        if (!bar->isHidden() && bar->sizeHint().height() > 0) groupTabsAvailable = true;
                    if (!groupTabsAvailable)
                    { finish(false, "native docking group tabs unavailable"); return; }
                    if (viewport->isFloating() || console->isHidden() ||
                        !window.restoreState(changedLayout, 2) || !viewport->isFloating() || !console->isHidden() ||
                        !window.restoreState(originalLayout, 2))
                    { finish(false, "dock layout round trip failed"); return; }
                }
                {
                    const auto root = check->Directory.filePath(QString::fromUtf8("project-\xe6\xb5\x8b\xe8\xaf\x95"));
                    const auto other = root + "-sibling";
                    if (!QDir().mkpath(root + "/Assets") || !QDir().mkpath(other))
                    { finish(false, "project fixture folders failed"); return; }
                    QFile file(root + "/Assets/sample.txt");
                    if (!file.open(QIODevice::WriteOnly) || file.write("asset") != 5)
                    { finish(false, "project fixture asset failed"); return; }
                    file.close();
                    auto& workspace = window.Workspace();
                    window.SetDraftText("Project switching preserves this draft.");
                    if (!window.OpenProject(root) || !workspace.Select(file.fileName()) ||
                        !window.InspectorText().contains("5 bytes"))
                    { finish(false, "project or resource metadata failed"); return; }
                    const auto generation = workspace.Generation();
                    const auto selected = workspace.SelectedPath();
                    if (window.OpenProject(root + "/missing") || workspace.Generation() != generation ||
                        workspace.SelectedPath() != selected || window.Document().Text() != "Project switching preserves this draft.")
                    { finish(false, "failed project open replaced state"); return; }
                    auto* tree = workspace.findChild<QTreeView*>("AssetsTree");
                    auto* model = tree ? qobject_cast<QFileSystemModel*>(tree->model()) : nullptr;
                    if (!model || !model->isReadOnly() || tree->rootIndex() != model->index(workspace.Root()) ||
                        !(model->filter() & QDir::NoSymLinks))
                    { finish(false, "resource model scope/read-only policy failed"); return; }
                    if (workspace.Select(root + "/../" + QFileInfo(other).fileName()) || !workspace.SelectedPath().isEmpty())
                    { finish(false, "outside-project selection accepted"); return; }
                    const auto assetIndex = model->index(file.fileName());
                    tree->setCurrentIndex(assetIndex);
                    if (!assetIndex.isValid() || workspace.SelectedPath() != selected ||
                        !window.InspectorText().contains("5 bytes") || !QFile::remove(file.fileName()))
                    { finish(false, "resource removal fixture failed"); return; }
                    workspace.ValidateSelection();
                    if (!workspace.SelectedPath().isEmpty() || window.InspectorText().contains("sample.txt"))
                    { finish(false, "removed resource selection survived"); return; }
                    std::error_code linkError;
                    const auto link = root + "/outside-link";
                    std::filesystem::create_directory_symlink(std::filesystem::path(other.toStdWString()),
                        std::filesystem::path(link.toStdWString()), linkError);
                    if (!linkError)
                    {
                        if (workspace.Select(link)) { finish(false, "symlink selection accepted"); return; }
                        std::filesystem::remove(std::filesystem::path(link.toStdWString()), linkError);
                        if (linkError) { finish(false, "symlink fixture cleanup failed"); return; }
                    }
                    else std::cout << "SKIP: symlink creation unavailable; canonical outside-root rejection is tested\n";
                    if (!workspace.Select(root + "/Assets") || !window.OpenProject(other) ||
                        workspace.Generation() == generation || !workspace.SelectedPath().isEmpty())
                    { finish(false, "project switch retained stale selection"); return; }
                    if (!QDir().rmdir(other)) { finish(false, "project removal fixture failed"); return; }
                    workspace.ValidateSelection();
                    if (!workspace.Root().isEmpty()) { finish(false, "removed project remained open"); return; }
                    if (!window.OpenProject(root)) { finish(false, "project reopen failed"); return; }
                    window.CloseProject();
                    if (!workspace.Root().isEmpty() || !workspace.SelectedPath().isEmpty() ||
                        window.Document().Text() != "Project switching preserves this draft.")
                    { finish(false, "project close changed draft or retained selection"); return; }
                    if (!window.OpenProject(root) || !workspace.Select(root + "/Assets"))
                    { finish(false, "project setup for crash recovery failed"); return; }
                    for (int switchCount = 0; switchCount < 20; ++switchCount)
                    {
                        if (!window.OpenProject(root)) { finish(false, "repeated project open failed"); return; }
                    }
                    if (workspace.findChildren<QFileSystemModel*>().size() != 1 ||
                        tree->findChildren<QItemSelectionModel*>().size() != 1 || !workspace.Select(root + "/Assets"))
                    {
                        std::cout << "Project models=" << workspace.findChildren<QFileSystemModel*>().size()
                            << ", selection models=" << tree->findChildren<QItemSelectionModel*>().size() << '\n';
                        finish(false, "project switch accumulated models or lost selection"); return;
                    }
                    QFile preview(root + "/README.txt");
                    if (!preview.open(QIODevice::WriteOnly) || preview.write("Project browser preview") != 23)
                    { finish(false, "project preview fixture failed"); return; }
                    preview.close();
                    auto* finalModel = qobject_cast<QFileSystemModel*>(tree->model());
                    tree->setCurrentIndex(finalModel->index(preview.fileName()));
                    if (!workspace.SelectedPath().endsWith("README.txt"))
                    { finish(false, "resource tree selection did not update inspector"); return; }
                }
                window.SetDraftText("Host draft must survive a child-process failure.");
                {
                    auto* tabs = window.findChild<QTabWidget*>("DraftTabs");
                    auto* active = tabs->currentWidget();
                    if (!window.NewDraft()) { finish(false, "background document setup failed"); return; }
                    check->BackgroundTab = tabs->currentWidget();
                    window.SetDraftText("Background draft survives engine failure.");
                    tabs->setCurrentWidget(active);
                }
                worker.Start(check->Directory.path() + "/missing-worker.exe");
                ++check->Phase;
                timer->start();
                break;
            case 1:
                if (state != State::Disconnected || worker.Running()) break;
                window.StartWorker(); ++check->Phase;
                break;
            case 2:
                if (state != State::Ready || !window.HasImage()) break;
                if (QCoreApplication::arguments().contains("--engine-check"))
                {
                    if (window.FrameCount() < 2) break;
                    if (!window.HasEngineEvidence())
                    { finish(false, "engine triangle pixels/orientation or console log missing"); return; }
                }
                if (!document.Dirty() || !document.CanUndo())
                { finish(false, "document history missing before crash"); return; }
                {
                    const auto args = QCoreApplication::arguments();
                    const auto index = args.indexOf("--self-test-image");
                    if (index >= 0 && index + 1 < args.size() && !window.grab().save(args[index + 1]))
                    { finish(false, "failed to render test image"); return; }
                }
                worker.Inject(Protocol::Kind::Crash); ++check->Phase;
                break;
            case 3:
                if (state != State::Disconnected || worker.Running()) break;
                if (document.Text() != "Host draft must survive a child-process failure." || !document.Dirty())
                { finish(false, "child failure changed host document"); return; }
                if (window.Workspace().Root().isEmpty() || window.Workspace().SelectedPath().isEmpty())
                { finish(false, "child failure changed host project selection"); return; }
                {
                    auto* tabs = window.findChild<QTabWidget*>("DraftTabs");
                    auto* active = tabs->currentWidget(); tabs->setCurrentWidget(check->BackgroundTab);
                    const bool retained = window.Document().Text() == "Background draft survives engine failure." && window.Document().Dirty();
                    tabs->setCurrentWidget(active);
                    if (!retained) { finish(false, "child failure changed inactive document"); return; }
                }
                if (!window.SaveDraft(check->Directory.filePath("recovered.gedraft")))
                { finish(false, "could not save while disconnected"); return; }
                document.Undo();
                if (!document.Dirty()) { finish(false, "undo did not leave saved state"); return; }
                document.Redo();
                if (document.Dirty()) { finish(false, "redo did not restore saved state"); return; }
                window.SetDraftText("Unsaved change after recovery");
                if (window.SaveDraft(check->Directory.filePath("missing/failed.gedraft")) || !document.Dirty())
                { finish(false, "failed save changed document state"); return; }
                {
                    QFile invalid(check->Directory.filePath("bad.gedraft"));
                    if (!invalid.open(QIODevice::WriteOnly) || invalid.write("not a document") != 14)
                    { finish(false, "could not prepare invalid input"); return; }
                }
                if (window.OpenDraft(check->Directory.filePath("bad.gedraft")) ||
                    document.Text() != "Unsaved change after recovery" || !document.Dirty())
                { finish(false, "invalid open replaced document"); return; }
                window.StartWorker(); ++check->Phase;
                break;
            case 4:
                if (state != State::Ready || !window.HasImage()) break;
                check->HangTick = check->UiTicks;
                worker.Inject(Protocol::Kind::Hang); ++check->Phase;
                break;
            case 5:
                if (state != State::Unresponsive) break;
                if (check->UiTicks - check->HangTick < 20 || !document.Dirty())
                { finish(false, "UI or draft did not survive worker hang"); return; }
                worker.Stop(); ++check->Phase;
                break;
            case 6:
                if (state != State::Stopped || worker.Running()) break;
                window.StartWorker(); ++check->Phase;
                break;
            case 7:
                if (state != State::Ready || !window.HasImage()) break;
                worker.Inject(Protocol::Kind::Malformed); ++check->Phase;
                break;
            case 8:
                if (state != State::Disconnected || worker.Running()) break;
                window.StartWorker(); ++check->Phase;
                break;
            case 9:
                if (state != State::Ready || !window.HasImage()) break;
                worker.Stop(); ++check->Phase;
                break;
            case 10:
                if (state != State::Stopped || worker.Running()) break;
                timer->stop();
                {
                    auto* tabs = window.findChild<QTabWidget*>("DraftTabs");
                    if (!window.OpenDraft(check->Directory.filePath("recovered.gedraft")) ||
                        !window.Document().Dirty() || window.Document().Text() != "Unsaved change after recovery")
                    { finish(false, "duplicate open lost unsaved recovered tab"); return; }
                    answerDialogs({QMessageBox::Discard});
                    if (!window.CloseDraft(tabs->currentIndex()))
                    { finish(false, "recovered tab discard failed"); return; }
                }
                if (!window.OpenDraft(check->Directory.filePath("recovered.gedraft")) || window.Document().Dirty() ||
                    window.Document().Text() != "Host draft must survive a child-process failure.")
                { finish(false, "saved draft round trip failed"); return; }
                finish(true, "independent document tabs/reorder/close-cancel, project/assets/inspector, docks/layout, worker failure recovery, undo and atomic save");
                break;
            }
        });
        timer->start();
    }
}
