#include "MainWindow.h"
#include <QAction>
#include <algorithm>
#include <array>
#include <iterator>
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
#include <QImageReader>
#include <QPainter>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QSettings>
#include <QStatusBar>
#include <QTemporaryFile>
#include <QTimer>
#include <QVBoxLayout>

namespace GEngineEditor
{
namespace
{
    bool ValidExecutableName(const QString& name)
    {
        if (name.isEmpty() || name.size() > 101 || name.endsWith('.') || name.endsWith(' ')) return false;
        for (const auto character : name)
        {
            if (character.unicode() < 32 || QString("<>:\"/\\|?*").contains(character)) return false;
        }
        const auto first = name.front();
        return first.isLetterOrNumber() || first == '_' || first == '-';
    }

    QImage RenderIconFrame(const QImage& source, int size)
    {
        QImage frame(size, size, QImage::Format_ARGB32);
        frame.fill(Qt::transparent);
        QPainter painter(&frame);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        const auto scale = std::min(
            static_cast<qreal>(size) / source.width(),
            static_cast<qreal>(size) / source.height());
        const auto width = static_cast<int>(source.width() * scale);
        const auto height = static_cast<int>(source.height() * scale);
        const QRect target((size - width) / 2, (size - height) / 2, width, height);
        painter.drawImage(target, source, source.rect());
        return frame;
    }

    void AppendUInt16(QIODevice& device, quint16 value)
    {
        const unsigned char bytes[] = {static_cast<unsigned char>(value & 0xff),
            static_cast<unsigned char>((value >> 8) & 0xff)};
        device.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
    }

    void AppendUInt32(QIODevice& device, quint32 value)
    {
        const unsigned char bytes[] = {static_cast<unsigned char>(value & 0xff),
            static_cast<unsigned char>((value >> 8) & 0xff),
            static_cast<unsigned char>((value >> 16) & 0xff),
            static_cast<unsigned char>((value >> 24) & 0xff)};
        device.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
    }

    QTemporaryFile* CreatePackageIcon(const QString& logoPath, QObject* parent)
    {
        QImageReader reader(logoPath);
        reader.setAutoTransform(true);
        const QImage source = reader.read();
        if (source.isNull() || source.width() <= 0 || source.height() <= 0 ||
            source.width() > 4096 || source.height() > 4096 ||
            source.sizeInBytes() > 64 * 1024 * 1024)
        {
            return nullptr;
        }
        static constexpr int sizes[] = {16, 24, 32, 48, 64, 128, 256};
        constexpr int count = static_cast<int>(std::size(sizes));
        std::array<QImage, count> frames;
        std::array<quint32, count> offsets{};
        quint32 payloadOffset = 6 + count * 16;
        auto* file = new QTemporaryFile(parent);
        file->setFileTemplate(QDir::tempPath() + "/GEngineIcon-XXXXXX.ico");
        if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            delete file;
            return nullptr;
        }
        AppendUInt16(*file, 0);
        AppendUInt16(*file, 1);
        AppendUInt16(*file, static_cast<quint16>(count));
        for (int index = 0; index < count; ++index)
        {
            frames[index] = RenderIconFrame(source, sizes[index]);
            const int maskStride = ((sizes[index] + 31) / 32) * 4;
            const quint32 imageSize = 40 + sizes[index] * sizes[index] * 4 +
                sizes[index] * maskStride;
            offsets[index] = payloadOffset;
            payloadOffset += imageSize;
            const auto dimension = sizes[index] == 256 ? 0 : sizes[index];
            AppendUInt16(*file, 0);
            AppendUInt16(*file, static_cast<quint8>(dimension));
            AppendUInt16(*file, static_cast<quint8>(dimension));
            AppendUInt16(*file, 0);
            AppendUInt16(*file, 1);
            AppendUInt16(*file, 32);
            AppendUInt32(*file, imageSize);
            AppendUInt32(*file, offsets[index]);
        }
        for (const auto& frame : frames)
        {
            const int size = frame.width();
            const int maskStride = ((size + 31) / 32) * 4;
            const quint32 imageSize = 40 + size * size * 4 + size * maskStride;
            AppendUInt32(*file, 40);
            AppendUInt32(*file, static_cast<quint32>(size));
            AppendUInt32(*file, static_cast<quint32>(size * 2));
            AppendUInt16(*file, 1);
            AppendUInt16(*file, 32);
            AppendUInt32(*file, 0);
            AppendUInt32(*file, imageSize);
            AppendUInt32(*file, 0);
            AppendUInt32(*file, 0);
            AppendUInt32(*file, 0);
            AppendUInt32(*file, 0);
            for (int row = size - 1; row >= 0; --row)
            {
                file->write(reinterpret_cast<const char*>(frame.scanLine(row)), size * 4);
            }
            file->write(QByteArray(size * maskStride, '\0'));
        }
        file->flush();
        file->close();
        if (file->error() != QFileDevice::NoError)
        {
            delete file;
            return nullptr;
        }
        return file;
    }
}

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
    bool MainWindow::SavePackaging(const QString& output, const QString& executableName,
        const QString& logoPath)
    {
        if (m_Project.Path.isEmpty() || PackagingRunning()) { m_Error = "Open a .gproject file before configuring packaging."; return false; }
        if (output.trimmed().isEmpty()) { m_Error = "Choose an output directory."; return false; }
        const auto destination = QDir::cleanPath(QDir(output).absolutePath());
        auto contains = [](const QString& parent, const QString& child)
        { return parent.compare(child, Qt::CaseInsensitive) == 0 || child.startsWith(parent + '/', Qt::CaseInsensitive); };
        if (contains(m_Project.Root(), destination) || contains(destination, m_Project.Root()))
        { m_Error = "Choose an output folder outside the project."; return false; }
        const auto executable = executableName.trimmed();
        if (!ValidExecutableName(executable)) { m_Error = "Choose a valid executable name."; return false; }
        const auto logo = logoPath.trimmed();
        QTemporaryFile* icon = nullptr;
        if (!logo.isEmpty())
        {
            const auto logoFile = QFileInfo(logo);
            if (!logoFile.isFile() || logoFile.isSymLink() || logoFile.size() > 64 * 1024 * 1024)
            { m_Error = "Choose a readable local image logo."; return false; }
            icon = CreatePackageIcon(logoFile.absoluteFilePath(), this);
            if (!icon) { m_Error = "The logo is not a supported image or is too large."; return false; }
        }
        m_PackageOutputDirectory = destination;
        m_PackageExecutableName = executable;
        m_PackageLogoPath = QFileInfo(logo).absoluteFilePath();
        delete m_PackageIconFile;
        m_PackageIconFile = icon;
        const auto key = QCryptographicHash::hash(m_Project.Path.toCaseFolded().toUtf8(), QCryptographicHash::Sha256).toHex();
        QSettings settings;
        settings.setValue("packagingOutputs/" + QString::fromLatin1(key), destination);
        settings.setValue("packagingExecutableNames/" + QString::fromLatin1(key), executable);
        settings.setValue("packagingLogos/" + QString::fromLatin1(key), m_PackageLogoPath);
        return true;
    }
    bool MainWindow::ShowPackagingDialog()
    {
        QDialog dialog(this); dialog.setObjectName("PackagingSettingsDialog"); dialog.setWindowTitle("Package project");
        auto* layout = new QVBoxLayout(&dialog);
        auto* note = new QLabel("The editor will compile your game and create its executable and resource pack in this folder.", &dialog);
        note->setWordWrap(true); layout->addWidget(note);
        auto* form = new QFormLayout; layout->addLayout(form);
        auto outputRow = [&](const QString& text, const char* id, const QString& value)
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
        auto* output = outputRow("Output directory", "PackageOutput", m_PackageOutputDirectory);
        auto* executable = new QLineEdit(m_PackageExecutableName, &dialog);
        executable->setObjectName("PackageExecutableName"); executable->setMinimumWidth(420);
        form->addRow("Executable name", executable);
        auto* logoRow = new QWidget(&dialog); auto* logoLayout = new QHBoxLayout(logoRow);
        logoLayout->setContentsMargins(0, 0, 0, 0);
        auto* logo = new QLineEdit(m_PackageLogoPath, logoRow); logo->setObjectName("PackageLogoPath");
        logo->setMinimumWidth(420);
        auto* browseLogo = new QPushButton("Browse...", logoRow); logoLayout->addWidget(logo); logoLayout->addWidget(browseLogo);
        form->addRow("Logo/icon image", logoRow);
        connect(browseLogo, &QPushButton::clicked, &dialog, [&, logo]
        {
            const auto selected = QFileDialog::getOpenFileName(&dialog, "Package logo", logo->text(),
                "Images (*.bmp *.gif *.jpg *.jpeg *.png *.svg *.tif *.tiff *.webp);;All files (*.*)");
            if (!selected.isEmpty()) logo->setText(selected);
        });
        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        buttons->button(QDialogButtonBox::Ok)->setText("Build and package"); layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, [&, output, executable, logo]
        {
            if (SavePackaging(output->text(), executable->text(), logo->text())) dialog.accept();
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
        if (!SavePackaging(m_PackageOutputDirectory, m_PackageExecutableName, m_PackageLogoPath)) return false;
        const auto exporter = QDir(QCoreApplication::applicationDirPath()).filePath("GEngineTools.exe");
        if (!QFileInfo::exists(exporter))
        { m_Error = "Packaging requires the native editor packaging tool. Rebuild/deploy the editor if it is missing."; return false; }
        m_Project = current; m_PackagingCancelled = false; m_PackagingOutput.clear(); m_Error.clear();
        QStringList args{"export-project", "--project", m_Project.Path, "--output", m_PackageOutputDirectory,
            "--executable-name", m_PackageExecutableName};
        if (m_PackageIconFile) args << "--icon" << m_PackageIconFile->fileName();
        m_PackagingProcess->setWorkingDirectory(m_Project.Root());
        m_PackagingProcess->start(exporter, args);
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
