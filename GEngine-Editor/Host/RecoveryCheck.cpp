#include "MainWindow.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QTemporaryDir>
#include <QTimer>
#include <QDockWidget>
#include <QAction>
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
        QObject::connect(timer, &QTimer::timeout, &window, [&window, check, finish]
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
                if (!check->Directory.isValid()) { finish(false, "temporary directory unavailable"); return; }
                {
                    const auto originalLayout = window.saveState(2);
                    for (const auto* name : {"HierarchyDock", "DocumentDock", "ConsoleDock", "ViewportDock"})
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
                    if (viewport->isFloating() || console->isHidden() ||
                        !window.restoreState(changedLayout, 2) || !viewport->isFloating() || !console->isHidden() ||
                        !window.restoreState(originalLayout, 2))
                    { finish(false, "dock layout round trip failed"); return; }
                }
                window.SetDraftText("Host draft must survive a child-process failure.");
                worker.Start(check->Directory.path() + "/missing-worker.exe");
                ++check->Phase;
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
                if (!window.OpenDraft(check->Directory.filePath("recovered.gedraft")) || document.Dirty() ||
                    document.Text() != "Host draft must survive a child-process failure.")
                { finish(false, "saved draft round trip failed"); return; }
                finish(true, "all dock panels/layout, missing child, handshake/image, crash, hang, malformed pipe, restart, undo and atomic save");
                break;
            }
        });
        timer->start();
    }
}
