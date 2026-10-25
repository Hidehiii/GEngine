#include "MainWindow.h"
#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QFont>
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
    const auto workerIndex = args.indexOf("--worker");
    if (workerIndex >= 0 && workerIndex + 1 >= args.size())
    { std::cerr << "--worker requires an executable path\n"; return 1; }
    const auto workerPath = workerIndex >= 0 ? args[workerIndex + 1] :
        QDir(application.applicationDirPath()).absoluteFilePath(
            args.contains("--self-test") ? "../GEngine-EditorDiagnosticWorker/GEngine-EditorDiagnosticWorker.exe" :
            "../GEngine-EditorEngineWorker/GEngine-EditorEngineWorker.exe");
    GEngineEditor::MainWindow window(workerPath);
    window.show();
    if (args.contains("--self-test")) GEngineEditor::RunRecoveryCheck(window);
    else QTimer::singleShot(0, &window, [&window] { window.StartWorker(); });
    return application.exec();
}
