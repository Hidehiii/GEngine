#include "MainWindow.h"
#include <QAction>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QSaveFile>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QSettings>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>

namespace GEngineEditor
{
    bool MainWindow::PackagingRunning() const
    { return m_PackagingProcess && m_PackagingProcess->state() != QProcess::NotRunning; }

    void MainWindow::RefreshPackagingActions()
    {
        if (!m_Package) return;
        const bool available = !m_Project.Path.isEmpty() && !m_Workspace->Root().isEmpty();
        const bool running = PackagingRunning();
        m_Package->setEnabled(available && !running);
        m_CancelPackaging->setEnabled(running && !m_PackagingCancelled);
        m_CloseProject->setEnabled(!m_Workspace->Root().isEmpty() && !running);
    }
    bool MainWindow::NewProject(const QString& path)
    {
        if (PackagingRunning() || QFileInfo::exists(path))
        { m_Error = "Choose a new project file, after packaging finishes."; return false; }
        ProjectFile project;
        project.Path = QFileInfo(path).absoluteFilePath();
        project.Name = QFileInfo(path).completeBaseName();
        if (!project.Save(m_Error)) return false;
        const auto sources = QDir(project.Root()).filePath("src");
        if (!QDir(sources).exists())
        {
            if (!QDir().mkpath(sources)) { m_Error = "Could not create the project source directory."; return false; }
            QSaveFile starter(QDir(sources).filePath("Game.cpp"));
            const QByteArray code = "#include <GEngine.h>\n#include <GEngine/EntryPoint.h>\nnamespace GEngine {\n"
                "Application* CreateApplication(ApplicationCommandLineArgs args) {\n"
                "    ApplicationSpecification spec; spec.Name = \"Game\"; spec.CommandLineArgs = args;\n"
                "    spec.EnableScripting = false; spec.EnablePhysics = false; spec.EnableOpenSSL = false; spec.EnableImGui = false;\n"
                "    return new Application(spec);\n}\n}\n";
            if (!starter.open(QIODevice::WriteOnly) || starter.write(code) != code.size() || !starter.commit())
            { m_Error = "Could not create the starter game source."; return false; }
        }
        return OpenProject(path);
    }
    bool MainWindow::SavePackaging(const QString& output)
    {
        if (m_Project.Path.isEmpty() || PackagingRunning()) { m_Error = "Open a .gproject file before configuring packaging."; return false; }
        if (output.trimmed().isEmpty()) { m_Error = "Choose an output directory."; return false; }
        const auto destination = QDir::cleanPath(QDir(output).absolutePath());
        auto contains = [](const QString& parent, const QString& child)
        { return parent.compare(child, Qt::CaseInsensitive) == 0 || child.startsWith(parent + '/', Qt::CaseInsensitive); };
        if (contains(m_Project.Root(), destination) || contains(destination, m_Project.Root()))
        { m_Error = "Choose an output folder outside the project."; return false; }
        m_PackageOutputDirectory = destination;
        const auto key = QCryptographicHash::hash(m_Project.Path.toCaseFolded().toUtf8(), QCryptographicHash::Sha256).toHex();
        QSettings settings;
        settings.setValue("packagingOutputs/" + QString::fromLatin1(key), destination);
        return true;
    }
    bool MainWindow::ShowPackagingDialog()
    {
        QDialog dialog(this); dialog.setObjectName("PackagingSettingsDialog"); dialog.setWindowTitle("Package project");
        auto* layout = new QVBoxLayout(&dialog);
        auto* note = new QLabel("The editor will compile your game and create its executable and resource pack in this folder.", &dialog);
        note->setWordWrap(true); layout->addWidget(note);
        auto* form = new QFormLayout; layout->addLayout(form);
        auto row = [&](const QString& text, const char* id, const QString& value)
        {
            auto* container = new QWidget(&dialog); auto* line = new QHBoxLayout(container); line->setContentsMargins(0, 0, 0, 0);
            auto* edit = new QLineEdit(value, container); edit->setObjectName(id); edit->setMinimumWidth(420);
            auto* browse = new QPushButton("Browse...", container); line->addWidget(edit); line->addWidget(browse); form->addRow(text, container);
            connect(browse, &QPushButton::clicked, &dialog, [&, edit]
            {
                const auto selected = QFileDialog::getExistingDirectory(&dialog, "Package output directory", edit->text());
                if (!selected.isEmpty()) edit->setText(selected);
            });
            return edit;
        };
        auto* output = row("Output directory", "PackageOutput", m_PackageOutputDirectory);
        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        buttons->button(QDialogButtonBox::Ok)->setText("Build and package"); layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, [&, output]
        {
            if (SavePackaging(output->text())) dialog.accept();
            else QMessageBox::warning(&dialog, "Packaging settings", m_Error);
        });
        return dialog.exec() == QDialog::Accepted;
    }
    bool MainWindow::PackageProject()
    {
        m_PackagingSucceeded = false;
        if (PackagingRunning() || m_Project.Path.isEmpty() || m_Workspace->Root().isEmpty())
        { m_Error = "Open a .gproject file and wait for the current export to finish."; return false; }
        ProjectFile current;
        if (!ProjectFile::Load(m_Project.Path, current, m_Error)) return false;
        if (m_PackageOutputDirectory.trimmed().isEmpty())
        { m_Error = "Choose an output directory."; return false; }
        const auto powershell = QStandardPaths::findExecutable("pwsh");
        const auto script = QDir(QCoreApplication::applicationDirPath()).filePath("packaging/ExportProject.ps1");
        if (powershell.isEmpty() || !QFileInfo::exists(script))
        { m_Error = "Packaging requires PowerShell 7.2+ and the editor packaging tools. Rebuild/deploy the editor if tools are missing."; return false; }
        m_Project = current; m_PackagingCancelled = false; m_PackagingOutput.clear(); m_Error.clear();
        QStringList args{"-NoLogo", "-NoProfile", "-NonInteractive", "-File", script, "-Project", m_Project.Path,
            "-Output", m_PackageOutputDirectory};
        m_PackagingProcess->setWorkingDirectory(m_Project.Root());
        m_PackagingProcess->start(powershell, args);
        m_PackagingDeadline->start(1800000);
        Log("Building and packaging project: " + m_Project.Name + "\nOutput: " + m_PackageOutputDirectory);
        RefreshPackagingActions(); return true;
    }
    void MainWindow::CancelPackaging()
    {
        if (!PackagingRunning()) return;
        m_PackagingCancelled = true; m_PackagingSucceeded = false;
        Log("Cancelling packaging. The previous package is preserved; an incomplete staging directory may remain.");
        m_PackagingProcess->kill(); RefreshPackagingActions();
    }
    void MainWindow::InitProjectActions(QMenu* file, QMenuBar* menus)
    {
        auto* create = new QAction("New project...", this); create->setObjectName("NewProjectAction");
        file->insertAction(file->actions().front(), create);
        connect(create, &QAction::triggered, this, [this]
        {
            auto path = QFileDialog::getSaveFileName(this, "New project", {}, "GEngine project (*.gproject)");
            if (path.isEmpty()) return;
            if (!path.endsWith(".gproject", Qt::CaseInsensitive)) path += ".gproject";
            if (!NewProject(path)) QMessageBox::warning(this, "New project failed", m_Error);
        });
        auto* open = new QAction("Open project...", this); open->setObjectName("OpenProjectFileAction");
        file->insertAction(m_CloseProject, open);
        connect(open, &QAction::triggered, this, [this]
        {
            const auto path = QFileDialog::getOpenFileName(this, "Open project", {}, "GEngine project (*.gproject)");
            if (!path.isEmpty() && !OpenProject(path)) QMessageBox::warning(this, "Open project failed", m_Error);
        });
        auto* menu = menus->addMenu("Project");
        m_Package = menu->addAction("Package project..."); m_Package->setObjectName("PackageProjectAction");
        m_CancelPackaging = menu->addAction("Cancel packaging"); m_CancelPackaging->setObjectName("CancelPackagingAction");
        connect(m_Package, &QAction::triggered, this, [this]
        { if (ShowPackagingDialog() && !PackageProject()) QMessageBox::warning(this, "Packaging failed", m_Error); });
        connect(m_CancelPackaging, &QAction::triggered, this, [this] { CancelPackaging(); });
        m_PackagingProcess = new QProcess(this); m_PackagingProcess->setProcessChannelMode(QProcess::MergedChannels);
        m_PackagingDeadline = new QTimer(this); m_PackagingDeadline->setSingleShot(true);
        connect(m_PackagingDeadline, &QTimer::timeout, this, [this] { Log("Build and packaging timed out after thirty minutes."); CancelPackaging(); });
        auto drain = [this]
        {
            const auto bytes = m_PackagingProcess->readAll();
            m_PackagingOutput += bytes.left(65536);
            int newline;
            while ((newline = m_PackagingOutput.indexOf('\n')) >= 0)
            { Log(QString::fromUtf8(m_PackagingOutput.left(newline).left(4096))); m_PackagingOutput.remove(0, newline + 1); }
            if (m_PackagingOutput.size() > 65536) { Log("Packaging output line truncated."); m_PackagingOutput.clear(); }
        };
        connect(m_PackagingProcess, &QProcess::readyRead, this, drain);
        connect(m_PackagingProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error)
        {
            if (error != QProcess::FailedToStart) return;
            m_PackagingDeadline->stop(); m_Error = m_PackagingProcess->errorString(); Log("Packaging failed: " + m_Error);
            RefreshPackagingActions();
            if (m_ClosePending) QTimer::singleShot(0, this, [this] { close(); });
        });
        connect(m_PackagingProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, drain](int code, QProcess::ExitStatus status)
        {
            drain();
            if (!m_PackagingOutput.isEmpty()) Log(QString::fromUtf8(m_PackagingOutput.left(4096)));
            m_PackagingOutput.clear(); m_PackagingDeadline->stop();
            m_PackagingSucceeded = !m_PackagingCancelled && status == QProcess::NormalExit && code == 0;
            if (!m_PackagingSucceeded) m_Error = m_PackagingCancelled ? "Packaging cancelled." : "Packaging failed. See Console for details.";
            Log(m_PackagingSucceeded ? "Package operation completed successfully." : m_Error);
            statusBar()->showMessage(m_PackagingSucceeded ? "Package operation completed" : m_Error, 10000);
            RefreshPackagingActions();
            if (m_ClosePending) QTimer::singleShot(0, this, [this] { close(); });
        });
        RefreshPackagingActions();
    }
}
