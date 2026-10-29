#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QFontDatabase>
#include <QFont>
#include <QSettings>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextStream>
#include <memory>
#include <iostream>

int main(int argc, char** argv)
{
    QApplication application(argc, argv);
    application.setOrganizationName("GEngine");
    application.setApplicationName("QtEditorHostPrototype");
    const auto loadBundledFont = [](const QString& path, std::unique_ptr<QTemporaryFile>& fontFile)
    {
        QFile input(path);
        if (!input.open(QIODevice::ReadOnly | QIODevice::Text)) return -1;
        QTextStream stream(&input);
        QString type;
        QString format;
        QString encoding;
        QByteArray encoded;
        bool payloadStarted = false;
        while (!stream.atEnd())
        {
            const auto line = stream.readLine().trimmed();
            if (!payloadStarted)
            {
                if (line == "schemaVersion: 1") continue;
                if (line.startsWith("type: ")) type = line.mid(6);
                else if (line.startsWith("format: ")) format = line.mid(8);
                else if (line.startsWith("payloadEncoding: ")) encoding = line.mid(17);
                else if (line.startsWith("payload: !!binary")) payloadStarted = true;
                else if (!line.isEmpty()) return -1;
            }
            else if (!line.isEmpty()) encoded.append(line.toLatin1());
        }
        if (type != "font" || format != "ttf" || encoding != "base64" || encoded.isEmpty()) return -1;
        const auto decoded = QByteArray::fromBase64(encoded, QByteArray::AbortOnBase64DecodingErrors);
        if (decoded.isEmpty()) return -1;
        fontFile = std::make_unique<QTemporaryFile>();
        if (!fontFile->open() || fontFile->write(decoded) != decoded.size()) return -1;
        fontFile->close();
        return QFontDatabase::addApplicationFont(fontFile->fileName());
    };
    std::unique_ptr<QTemporaryFile> bundledFont;
    const auto fontPath = QDir(application.applicationDirPath()).filePath("fonts/NotoSansSC-Regular.gasset");
    const int fontId = loadBundledFont(fontPath, bundledFont);
    if (fontId >= 0)
    {
        const auto families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) application.setFont(QFont(families.front(), 10));
    }
    else std::cerr << "Bundled UI font could not be loaded: " << fontPath.toStdString() << '\n';
    const auto args = application.arguments();
    std::unique_ptr<QTemporaryDir> testSettings;
    if (args.contains("--self-test") || args.contains("--packaging-self-test") || args.contains("--project-settings-self-test") || args.contains("--import-self-test"))
    {
        testSettings = std::make_unique<QTemporaryDir>();
        if (!testSettings->isValid()) { std::cerr << "Could not isolate editor test settings\n"; return 1; }
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, testSettings->path());
    }
    const auto projectIndex = args.indexOf("--project");
    if (projectIndex >= 0 && projectIndex + 1 >= args.size())
    { std::cerr << "--project requires a .gproject file\n"; return 1; }
    const auto workerIndex = args.indexOf("--worker");
    if (workerIndex >= 0 && workerIndex + 1 >= args.size())
    { std::cerr << "--worker requires an executable path\n"; return 1; }
    const auto workerPath = workerIndex >= 0 ? args[workerIndex + 1] :
        QDir(application.applicationDirPath()).absoluteFilePath(
            (args.contains("--self-test") || args.contains("--project-settings-self-test")) ? "../GEngine-EditorDiagnosticWorker/GEngine-EditorDiagnosticWorker.exe" :
            "../GEngine-EditorEngineWorker/GEngine-EditorEngineWorker.exe");
    GEngineEditor::MainWindow window(workerPath);
    QString projectPath;
    if (projectIndex >= 0) projectPath = args[projectIndex + 1];
    else if (args.size() > 1 && args[1].endsWith(".gproject", Qt::CaseInsensitive)) projectPath = args[1];
    if (!projectPath.isEmpty() && !window.OpenProject(projectPath))
    { std::cerr << window.LastError().toStdString() << '\n'; return 1; }
    window.show();
    if (args.contains("--import-self-test")) GEngineEditor::RunImportCheck(window);
    else if (args.contains("--project-settings-self-test")) GEngineEditor::RunProjectSettingsCheck(window);
    else if (args.contains("--packaging-self-test")) GEngineEditor::RunPackagingCheck(window);
    else if (args.contains("--self-test")) GEngineEditor::RunRecoveryCheck(window);
    else QTimer::singleShot(0, &window, [&window] { window.StartWorker(); });
    return application.exec();
}
