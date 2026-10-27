#include "MainWindow.h"
#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
#include <iostream>
#include <memory>

namespace GEngineEditor
{
    void RunPackagingCheck(MainWindow& window)
    {
        struct Check { QTemporaryDir Directory; QElapsedTimer Clock; QString Project, Output; QByteArray Manifest, Metadata; int Phase = 0, Ticks = 0; };
        auto check = std::make_shared<Check>(); check->Clock.start();
        auto* timer = new QTimer(&window); timer->setInterval(20);
        auto finish = [timer](bool ok, const char* text)
        { timer->stop(); std::cout << (ok ? "PASS: " : "FAIL: ") << text << std::endl; QApplication::exit(ok ? 0 : 1); };
        auto read = [](const QString& path) { QFile file(path); return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray{}; };
        QObject::connect(timer, &QTimer::timeout, &window, [&window, timer, check, finish, read]
        {
            ++check->Ticks;
            if (check->Clock.elapsed() > 1700000) { finish(false, "automatic packaging deadline exceeded"); return; }
            if (check->Phase && window.PackagingRunning()) return;
            switch (check->Phase)
            {
            case 0:
            {
                timer->stop();
                const auto metadataPath = check->Directory.filePath("Metadata.gproject");
                if (!window.NewProject(metadataPath) || window.NewProject(metadataPath)) { finish(false, "new project handling failed"); return; }
                const auto metadata = QJsonDocument::fromJson(read(metadataPath)).object();
                if (metadata.size() != 2 || !metadata.contains("name") || !metadata.contains("schemaVersion") ||
                    !QFileInfo::exists(check->Directory.filePath("src/Game.cpp")))
                { finish(false, "project contains packaging fields or lacks starter source"); return; }
                ProjectFile invalid; invalid.Path=check->Directory.filePath("missing/Bad.gproject"); invalid.Name="Bad"; QString error;
                if (invalid.Save(error) || window.OpenProject(check->Directory.filePath("missing.gproject")))
                { finish(false, "invalid project/save changed state"); return; }
                auto root = QCoreApplication::applicationDirPath();
                while (!root.isEmpty() && !QFileInfo::exists(QDir(root).filePath("BuildScripts/Project/CMakeLists.txt")))
                { const auto parent = QFileInfo(root).dir().absolutePath(); if (parent == root) { root.clear(); break; } root=parent; }
                check->Project = QDir(root).filePath("Example/FrameGraphTriangle/FrameGraphTriangle.gproject");
                check->Output = check->Directory.filePath("output");
                if (root.isEmpty() || !window.OpenProject(check->Project)) { finish(false, "sample project unavailable"); return; }
                check->Metadata = read(check->Project);
                if (window.findChild<QAction*>("PreviewPackageAction") || window.findChild<QAction*>("PackagingSettingsAction") ||
                    window.SavePackaging(QFileInfo(check->Project).absolutePath()))
                { finish(false, "unexpected packaging controls or invalid output accepted"); return; }
                QTimer::singleShot(0, &window, [&window, check, finish]
                {
                    auto* dialog = window.findChild<QDialog*>("PackagingSettingsDialog");
                    if (!dialog || dialog->findChildren<QLineEdit*>().size() != 1)
                    { finish(false, "packaging must have exactly one output field"); return; }
                    dialog->findChild<QLineEdit*>("PackageOutput")->setText(check->Output);
                    const auto screenshot=qEnvironmentVariable("GENGINE_EDITOR_PACKAGING_SCREENSHOT");
                    if (!screenshot.isEmpty()) dialog->grab().save(screenshot);
                    dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Ok)->click();
                });
                window.findChild<QAction*>("PackageProjectAction")->trigger();
                if (!window.PackagingRunning() || window.OpenProject(check->Directory.path()))
                { finish(false, "build was not asynchronous or allowed project replacement"); return; }
                check->Phase=1; timer->start(); break;
            }
            case 1:
                if (!window.LastPackagingSucceeded() || read(check->Project) != check->Metadata ||
                    !QFileInfo::exists(check->Output+"/FrameGraphTriangle.exe") ||
                    !read(check->Output+"/FrameGraphTriangle.gpak").startsWith("GEPAK001") ||
                    QFileInfo::exists(check->Output+"/Assets") || check->Ticks < 2)
                { finish(false, "automatic build/flat EXE and resource pack failed or changed project metadata"); return; }
                check->Manifest=read(check->Output+"/.gengine-output.json");
                if (!window.OpenProject(check->Project) || !window.PackageProject())
                { finish(false, "output preference/reopen/repeat failed"); return; }
                ++check->Phase; break;
            case 2:
            {
                if (!window.LastPackagingSucceeded() || read(check->Output+"/.gengine-output.json") != check->Manifest ||
                    !window.findChild<QPlainTextEdit*>("ConsoleOutput")->toPlainText().contains("\"copyFiles\":0"))
                { finish(false, "incremental automatic rebuild/repeat failed"); return; }
                const auto brokenRoot=check->Directory.filePath("broken"); QDir().mkpath(brokenRoot);
                ProjectFile broken; broken.Path=brokenRoot+"/Broken.gproject"; broken.Name="Broken"; QString error;
                if (!broken.Save(error) || !window.OpenProject(broken.Path) || !window.SavePackaging(check->Output) || !window.PackageProject())
                { finish(false, "could not initiate failing source build"); return; }
                ++check->Phase; break;
            }
            case 3:
                if (window.LastPackagingSucceeded() || read(check->Output+"/.gengine-output.json") != check->Manifest ||
                    !window.OpenProject(check->Project) || !window.SavePackaging(check->Output) || !window.PackageProject())
                { finish(false, "compiler failure damaged existing output or recovery failed"); return; }
                QTimer::singleShot(600,&window,[&window] { window.CancelPackaging(); }); ++check->Phase; break;
            case 4:
                if (window.LastPackagingSucceeded() || !window.PackageProject()) { finish(false, "cancel/retry failed"); return; }
                ++check->Phase; break;
            case 5:
                if (!window.LastPackagingSucceeded()) { finish(false, "build did not recover after cancel"); return; }
                window.CloseProject();
                if (window.findChild<QAction*>("PackageProjectAction")->isEnabled()) { finish(false, "closed project can still package"); return; }
                finish(true,"metadata-only .gproject, output-only dialog, real automatic Release build and .gpak, incremental repeat, build failure preservation, cancel/retry, local output and project close");
                break;
            }
        });
        timer->start();
    }
}
