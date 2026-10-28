#include "MainWindow.h"
#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QFont>
#include <QSettings>
#include <QTemporaryDir>
#include <memory>
#include <iostream>

int main(int argc, char** argv)
{
    QApplication application(argc, argv);
    application.setOrganizationName("GEngine");
    application.setApplicationName("QtEditorHostPrototype");
    const auto fontPath = QDir(application.applicationDirPath()).filePath("fonts/NotoSansSC-Regular.ttf");
    const int fontId = QFontDatabase::addApplicationFont(fontPath);
    if (fontId >= 0)
    {
        const auto families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) application.setFont(QFont(families.front(), 10));
    }
    else std::cerr << "Bundled UI font could not be loaded: " << fontPath.toStdString() << '\n';
    const auto args = application.arguments();
    std::unique_ptr<QTemporaryDir> testSettings;
    if (args.contains("--self-test") || args.contains("--packaging-self-test") || args.contains("--project-settings-self-test"))
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
    if (args.contains("--project-settings-self-test")) GEngineEditor::RunProjectSettingsCheck(window);
    else if (args.contains("--packaging-self-test")) GEngineEditor::RunPackagingCheck(window);
    else if (args.contains("--self-test")) GEngineEditor::RunRecoveryCheck(window);
    else QTimer::singleShot(0, &window, [&window] { window.StartWorker(); });
    return application.exec();
}
