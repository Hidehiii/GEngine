#include "MainWindow.h"
#include <QAction>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDockWidget>
#include <QFile>
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
#include <iostream>

namespace GEngineEditor
{
    class ViewportWidget : public QWidget
    {
    public:
        QImage Image;
        QString State;
        bool Engine = false;
        explicit ViewportWidget(QWidget* parent = nullptr) : QWidget(parent) { setMinimumSize(320, 240); }
    protected:
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
                (Engine ? "GEngine GPU preview\n" : "CPU protocol diagnostic image - NOT an engine scene\n") + State);
        }
    };
    MainWindow::MainWindow(QString workerPath) : m_WorkerPath(std::move(workerPath))
    {
        resize(1280, 800);
        setObjectName("EditorHost");
        setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks |
            QMainWindow::AnimatedDocks | QMainWindow::GroupedDragging);
        auto* viewMenu = menuBar()->addMenu("View");
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
        m_Notes = new QPlainTextEdit(this);
        m_Notes->setUndoRedoEnabled(false);
        m_Notes->setPlaceholderText("Host-owned draft notes. This is not a .GEScene file.");
        m_Notes->setPlainText(QString::fromStdString(m_Document.Text()));
        dock("Document draft (not scene data)", "DocumentDock", m_Notes, Qt::RightDockWidgetArea);
        m_Console = new QPlainTextEdit(this);
        m_Console->setReadOnly(true);
        m_Console->setMaximumBlockCount(300);
        dock("Console", "ConsoleDock", m_Console, Qt::BottomDockWidgetArea);
        m_Viewport = new ViewportWidget(this);
        dock("Viewport", "ViewportDock", m_Viewport, Qt::LeftDockWidgetArea);
        m_StateLabel = new QLabel(this);
        statusBar()->addWidget(m_StateLabel, 1);
        auto* file = menuBar()->addMenu("File");
        auto* fresh = file->addAction("New draft");
        fresh->setShortcut(QKeySequence::New);
        connect(fresh, &QAction::triggered, this, [this]
        {
            if (!ConfirmDiscard()) return;
            m_Document.Reset("Untitled editor draft"); m_DraftPath.clear(); RefreshDocument();
        });
        auto* open = file->addAction("Open draft...");
        open->setShortcut(QKeySequence::Open);
        connect(open, &QAction::triggered, this, [this]
        {
            if (!ConfirmDiscard()) return;
            const auto path = QFileDialog::getOpenFileName(this, "Open editor draft", {}, "Editor draft (*.gedraft)");
            if (!path.isEmpty() && !OpenDraft(path)) QMessageBox::warning(this, "Open failed", m_Error);
        });
        auto* save = file->addAction("Save draft");
        save->setShortcut(QKeySequence::Save);
        connect(save, &QAction::triggered, this, [this]
        {
            if (m_DraftPath.isEmpty()) SaveAs();
            else if (!SaveDraft(m_DraftPath)) QMessageBox::warning(this, "Save failed", m_Error);
        });
        auto* saveAs = file->addAction("Save draft as...");
        saveAs->setShortcut(QKeySequence::SaveAs);
        connect(saveAs, &QAction::triggered, this, [this] { SaveAs(); });
        connect(file->addAction("Exit"), &QAction::triggered, this, &QWidget::close);
        auto* edit = menuBar()->addMenu("Edit");
        m_Undo = edit->addAction("Undo"); m_Undo->setShortcut(QKeySequence::Undo);
        m_Redo = edit->addAction("Redo"); m_Redo->setShortcut(QKeySequence::Redo);
        connect(m_Undo, &QAction::triggered, this, [this] { m_Document.Undo(); RefreshDocument(); });
        connect(m_Redo, &QAction::triggered, this, [this] { m_Document.Redo(); RefreshDocument(); });
        auto* workerMenu = menuBar()->addMenu("Engine");
        menuBar()->removeAction(viewMenu->menuAction());
        menuBar()->insertMenu(workerMenu->menuAction(), viewMenu);
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
        connect(m_Notes, &QPlainTextEdit::textChanged, this, [this]
        {
            const auto text = m_Notes->toPlainText().toUtf8();
            if (text.size() > 64 * 1024) { Log("Draft size limit is 64 KiB UTF-8."); RefreshDocument(); return; }
            m_Document.Edit(text.toStdString());
            RefreshDocument();
        });
        m_Worker.OnLog = [this](const QString& text) { Log(text); };
        m_Worker.OnState = [this](WorkerSupervisor::State)
        {
            RefreshWorker();
            if (m_ClosePending && !m_Worker.Running()) QTimer::singleShot(0, this, [this] { close(); });
        };
        m_Worker.OnFrame = [this](const QImage& image)
        { m_HasImage = true; ++m_FrameCount; m_Viewport->Engine = m_Worker.IsEngine(); m_Viewport->Image = image; m_Viewport->update(); };
        bool restored = false;
        if (!QCoreApplication::arguments().contains("--self-test"))
        {
            QSettings settings;
            restoreGeometry(settings.value("hostGeometry").toByteArray());
            restored = restoreState(settings.value("hostLayout").toByteArray(), 2);
        }
        if (!restored) QTimer::singleShot(0, this, [this] { ResetLayout(); });
        Log("Qt Host: Engine menu starts the render worker. Scene editing/Play/Gizmo are not implemented.");
        RefreshDocument(); RefreshWorker();
    }
    MainWindow::~MainWindow()
    {
        m_Worker.OnState = {}; m_Worker.OnLog = {}; m_Worker.OnFrame = {};
    }
    void MainWindow::ResetLayout()
    {
        auto* hierarchy = findChild<QDockWidget*>("HierarchyDock");
        auto* document = findChild<QDockWidget*>("DocumentDock");
        auto* console = findChild<QDockWidget*>("ConsoleDock");
        auto* viewport = findChild<QDockWidget*>("ViewportDock");
        for (auto* panel : {hierarchy, document, console, viewport})
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
        for (auto* panel : {hierarchy, document, console, viewport}) panel->show();
        resizeDocks({hierarchy, viewport, document}, {220, 740, 300}, Qt::Horizontal);
        resizeDocks({viewport, console}, {580, 180}, Qt::Vertical);
        auto* toolbar = findChild<QToolBar*>("WorkerToolbar");
        addToolBar(Qt::TopToolBarArea, toolbar);
        toolbar->show();
    }
    void MainWindow::Log(const QString& text)
    {
        const auto bounded = text.left(16 * 1024).trimmed();
        m_Console->appendPlainText(bounded);
        if (QCoreApplication::arguments().contains("--self-test"))
            std::cerr << bounded.toStdString() << '\n';
    }
    void MainWindow::SetDraftText(const QString& text) { m_Notes->setPlainText(text); }
    void MainWindow::RefreshDocument()
    {
        const auto text = QString::fromStdString(m_Document.Text());
        if (text != m_Notes->toPlainText()) { QSignalBlocker blocker(m_Notes); m_Notes->setPlainText(text); }
        m_Undo->setEnabled(m_Document.CanUndo()); m_Redo->setEnabled(m_Document.CanRedo());
        setWindowTitle(QString("GEngine Qt Editor - draft%1 - E0.1 prototype").arg(m_Document.Dirty() ? " *" : ""));
    }
    void MainWindow::RefreshWorker()
    {
        const auto description = m_Worker.Description();
        m_StateLabel->setText(description);
        m_Viewport->State = description; m_Viewport->update();
        m_Start->setEnabled(!m_Worker.Running() && m_Worker.GetState() != WorkerSupervisor::State::Starting);
        m_Stop->setEnabled(m_Worker.Running() || m_Worker.GetState() == WorkerSupervisor::State::Starting);
        const bool ready = m_Worker.GetState() == WorkerSupervisor::State::Ready;
        m_Crash->setEnabled(ready); m_Hang->setEnabled(ready); m_Malformed->setEnabled(ready);
    }
    void MainWindow::StartWorker()
    {
        if (m_Worker.Running()) return;
        m_HasImage = false; m_FrameCount = 0; m_Viewport->Image = {};
        m_Worker.Start(m_WorkerPath);
    }
    bool MainWindow::HasEngineEvidence() const
    {
        const auto& image = m_Viewport->Image;
        if (!m_Worker.IsEngine() || image.size() != QSize(160, 90)) return false;
        const auto center = image.pixelColor(80, 45);
        const auto corner = image.pixelColor(2, 2);
        return center.blue() > 230 && center.green() > 150 && center.red() < 80 &&
            corner.blue() < 100 && image.pixelColor(80, 20).blue() > 230 &&
            image.pixelColor(80, 75).blue() < 100 &&
            m_Console->toPlainText().contains("Editor engine frame 1 delivered");
    }
    bool MainWindow::SaveDraft(const QString& path)
    {
        const QJsonObject object{{"format", "GEngine.EditorDraft"}, {"version", 1},
            {"notes", QString::fromStdString(m_Document.Text())}};
        const auto bytes = QJsonDocument(object).toJson();
        QSaveFile file(path);
        file.setDirectWriteFallback(false);
        if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        { m_Error = file.errorString(); return false; }
        m_Error.clear(); m_DraftPath = path; m_Document.MarkSaved(); RefreshDocument();
        return true;
    }
    bool MainWindow::OpenDraft(const QString& path)
    {
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
        m_Error.clear(); m_DraftPath = path;
        m_Document.Reset(object.value("notes").toString().toUtf8().toStdString());
        RefreshDocument();
        return true;
    }
    bool MainWindow::SaveAs()
    {
        auto path = QFileDialog::getSaveFileName(this, "Save editor draft", m_DraftPath, "Editor draft (*.gedraft)");
        if (path.isEmpty()) return false;
        if (!path.endsWith(".gedraft", Qt::CaseInsensitive)) path += ".gedraft";
        if (SaveDraft(path)) return true;
        QMessageBox::warning(this, "Save failed", m_Error);
        return false;
    }
    bool MainWindow::ConfirmDiscard()
    {
        if (!m_Document.Dirty()) return true;
        const auto answer = QMessageBox::question(this, "Unsaved editor draft", "Save draft before continuing?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (answer == QMessageBox::Cancel) return false;
        if (answer == QMessageBox::Discard) return true;
        if (m_DraftPath.isEmpty()) return SaveAs();
        if (SaveDraft(m_DraftPath)) return true;
        QMessageBox::warning(this, "Save failed", m_Error);
        return false;
    }
    void MainWindow::closeEvent(QCloseEvent* event)
    {
        if (!m_ClosePending && !ConfirmDiscard()) { event->ignore(); return; }
        m_ClosePending = true;
        setEnabled(false);
        if (m_Worker.Running()) { event->ignore(); m_Worker.Stop(); return; }
        if (!QCoreApplication::arguments().contains("--self-test"))
        {
            QSettings settings;
            settings.setValue("hostGeometry", saveGeometry());
            settings.setValue("hostLayout", saveState(2));
        }
        event->accept();
    }
}
