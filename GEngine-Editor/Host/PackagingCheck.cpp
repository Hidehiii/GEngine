#include "MainWindow.h"
#include <windows.h>
#include <shellapi.h>

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QImage>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTemporaryDir>
#include <QTimer>
#include <iostream>
#include <memory>

namespace GEngineEditor
{
    namespace
    {
        bool HasEmbeddedIcon(const QString& executable)
        {
            HICON icon = nullptr;
            if (ExtractIconExW(reinterpret_cast<LPCWSTR>(executable.utf16()), 0, nullptr, &icon, 1) != 1 ||
                !icon)
            {
                return false;
            }
            ICONINFO information{};
            bool valid = GetIconInfo(icon, &information) && information.hbmColor;
            if (valid)
            {
                BITMAP bitmap{};
                valid = GetObject(information.hbmColor, sizeof(bitmap), &bitmap) != 0;
                if (valid)
                {
                    QImage pixels(bitmap.bmWidth, bitmap.bmHeight, QImage::Format_ARGB32);
                    BITMAPINFO format{};
                    format.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                    format.bmiHeader.biWidth = pixels.width();
                    format.bmiHeader.biHeight = -pixels.height();
                    format.bmiHeader.biPlanes = 1;
                    format.bmiHeader.biBitCount = 32;
                    format.bmiHeader.biCompression = BI_RGB;
                    HDC context = GetDC(nullptr);
                    valid = context && GetDIBits(context, information.hbmColor, 0,
                        pixels.height(), pixels.bits(), &format, DIB_RGB_COLORS) == pixels.height();
                    if (context) ReleaseDC(nullptr, context);
                    if (valid)
                    {
                        const auto corner = pixels.pixelColor(0, 0);
                        const auto center = pixels.pixelColor(pixels.width() / 2, pixels.height() / 2);
                        valid = corner.blue() > 150 && corner.red() < 60 &&
                            center.red() > 200 && center.green() > 200 && center.blue() > 200;
                    }
                }
            }
            if (information.hbmColor) DeleteObject(information.hbmColor);
            if (information.hbmMask) DeleteObject(information.hbmMask);
            DestroyIcon(icon);
            return valid;
        }

        QString WriteTestLogo(const QString& path)
        {
            QImage logo(64, 64, QImage::Format_ARGB32);
            logo.fill(QColor(18, 76, 204, 255));
            QPainter painter(&logo);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(QColor(236, 245, 255, 255));
            painter.drawEllipse(8, 8, 48, 48);
            return logo.save(path, "PNG") ? path : QString{};
        }
    }

    void RunPackagingCheck(MainWindow& window)
    {
        struct Check { QTemporaryDir Directory; QElapsedTimer Clock; QString Project, Output, Executable, Logo; QByteArray Manifest, Metadata, GameOutput, PreservedConfig; QProcess* Game = nullptr; int Phase = 0, Ticks = 0; };
        auto check = std::make_shared<Check>(); check->Clock.start();
        auto* timer = new QTimer(&window); timer->setInterval(20);
        auto finish = [timer, &window](bool ok, const char* text)
        {
            timer->stop();
            if (!ok)
                std::cerr << window.findChild<QPlainTextEdit*>("ConsoleOutput")->toPlainText().toStdString() << std::endl;
            std::cout << (ok ? "PASS: " : "FAIL: ") << text << std::endl;
            QApplication::exit(ok ? 0 : 1);
        };
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
                if (metadata.size() != 3 || !metadata.contains("name") || !metadata.contains("schemaVersion") ||
                    metadata.value("settings").toObject().value("defaultScene").toString() != "Assets/Scenes/Default.gscene" ||
                    !QFileInfo::exists(check->Directory.filePath("Assets/Scenes/Default.gscene")) ||
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
                check->Executable = check->Directory.filePath("TriangleGame.exe");
                check->Executable = QFileInfo(check->Executable).completeBaseName();
                check->Logo = WriteTestLogo(check->Directory.filePath("logo.png"));
                if (root.isEmpty() || !window.OpenProject(check->Project)) { finish(false, "sample project unavailable"); return; }
                check->Metadata = read(check->Project);
                if (window.findChild<QAction*>("PreviewPackageAction") || window.findChild<QAction*>("PackagingSettingsAction") ||
                    window.SavePackaging(check->Output, "Bad/Name", check->Logo) ||
                    window.SavePackaging(check->Output, check->Executable, check->Directory.filePath("missing.png")) ||
                    window.SavePackaging(QFileInfo(check->Project).absolutePath(), check->Executable, check->Logo))
                { finish(false, "unexpected packaging controls or invalid output accepted"); return; }
                QTimer::singleShot(0, &window, [&window, check, finish]
                {
                    auto* dialog = window.findChild<QDialog*>("PackagingSettingsDialog");
                    if (!dialog || dialog->findChildren<QLineEdit*>().size() != 3)
                    { finish(false, "packaging must have exactly output, executable-name and logo fields"); return; }
                    dialog->findChild<QLineEdit*>("PackageOutput")->setText(check->Output);
                    dialog->findChild<QLineEdit*>("PackageExecutableName")->setText(check->Executable);
                    dialog->findChild<QLineEdit*>("PackageLogoPath")->setText(check->Logo);
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
            {
                if (!window.LastPackagingSucceeded() || read(check->Project) != check->Metadata ||
                    !QFileInfo::exists(check->Output+"/"+check->Executable+".exe") ||
                    !read(check->Output+"/Data/"+check->Executable+".gpak").startsWith("GEPAK001") ||
                    !QFileInfo::exists(check->Output+"/Runtime/"+check->Executable+".exe") ||
                    !QFileInfo::exists(check->Output+"/Metadata/.gengine-output.json") ||
                    !QFileInfo::exists(check->Output+"/Config/Config.ini") ||
                    QFileInfo::exists(check->Output+"/FrameGraphTriangle.exe") ||
                    QFileInfo::exists(check->Output+"/FrameGraphTriangle.gpak") ||
                    QFileInfo::exists(check->Output+"/Assets") || !HasEmbeddedIcon(check->Output+"/"+check->Executable+".exe") ||
                    check->Ticks < 2)
                { finish(false, "automatic build/single-entry categorized output failed or changed project metadata"); return; }
                check->Manifest=read(check->Output+"/Metadata/.gengine-output.json");
                const auto manifest = QJsonDocument::fromJson(check->Manifest).object();
                if (manifest.value("name").toString() != check->Executable)
                { finish(false, "custom executable identity was not applied to the manifest"); return; }
                if (!window.OpenProject(check->Project) || !window.PackageProject())
                { finish(false, "output preference/reopen/repeat failed"); return; }
                ++check->Phase; break;
            }
            case 2:
            {
                if (!window.LastPackagingSucceeded())
                { finish(false, "incremental automatic rebuild failed"); return; }
                if (read(check->Output+"/Metadata/.gengine-output.json") != check->Manifest)
                { finish(false, "incremental manifest changed"); return; }
                if (!window.findChild<QPlainTextEdit*>("ConsoleOutput")->toPlainText().contains("\"copyFiles\":0"))
                { finish(false, "incremental repeat was not zero-copy"); return; }
                const auto brokenRoot=check->Directory.filePath("broken"); QDir().mkpath(brokenRoot);
                ProjectFile broken; broken.Path=brokenRoot+"/Broken.gproject"; broken.Name="Broken"; QString error;
                if (!broken.Save(error) || !window.OpenProject(broken.Path) ||
                    !window.SavePackaging(check->Output, check->Executable, check->Logo) || !window.PackageProject())
                { finish(false, "could not initiate failing source build"); return; }
                ++check->Phase; break;
            }
            case 3:
                if (window.LastPackagingSucceeded() || read(check->Output+"/Metadata/.gengine-output.json") != check->Manifest ||
                    !window.OpenProject(check->Project) ||
                    !window.SavePackaging(check->Output, check->Executable, check->Logo) || !window.PackageProject())
                { finish(false, "compiler failure damaged existing output or recovery failed"); return; }
                QTimer::singleShot(600,&window,[&window] { window.CancelPackaging(); }); ++check->Phase; break;
            case 4:
                if (window.LastPackagingSucceeded() || !window.PackageProject()) { finish(false, "cancel/retry failed"); return; }
                ++check->Phase; break;
            case 5:
                if (!window.LastPackagingSucceeded()) { finish(false, "build did not recover after cancel"); return; }
                check->Game = new QProcess(&window);
                check->Game->setProcessChannelMode(QProcess::MergedChannels);
                {
                    auto environment = QProcessEnvironment::systemEnvironment();
                    environment.insert("GENGINE_EXAMPLE_FRAME_LIMIT", "120");
                    environment.insert("GENGINE_EXAMPLE_RESOURCE_CHECK", "1");
                    environment.insert("GENGINE_EXAMPLE_RESOURCE_CHECK", "1");
                    environment.remove("GENGINE_EXAMPLE_SECONDS");
                    check->Game->setProcessEnvironment(environment);
                }
                check->Game->setWorkingDirectory(check->Directory.path());
                check->Game->start(check->Output + "/" + check->Executable + ".exe", QStringList{});
                ++check->Phase; break;
            case 6:
                check->GameOutput += check->Game->readAll();
                if (check->Game->state() != QProcess::NotRunning) return;
                if (check->Game->exitStatus() != QProcess::NormalExit || check->Game->exitCode() != 0 ||
                    check->GameOutput.count("readback passed after 119 frames.") != 3)
                {
                    std::cerr << check->GameOutput.toStdString() << std::endl;
                    finish(false, "custom launcher failed packaged 120-frame readbacks from unrelated cwd"); return;
                }
                {
                    QFile config(check->Output + "/Config/Config.ini");
                    const QByteArray comment("\n# preserve root configuration\n");
                    if (!config.open(QIODevice::WriteOnly | QIODevice::Append) || config.write(comment) != comment.size())
                    { finish(false, "could not prepare mutable config preservation check"); return; }
                    config.close();
                    check->PreservedConfig = read(check->Output + "/Config/Config.ini");
                    QFile cache(check->Output + "/Cache/keep.txt");
                    if (!cache.open(QIODevice::WriteOnly) || cache.write("persistent cache") != 16)
                    { finish(false, "could not prepare cache preservation check"); return; }
                }
                check->Executable = "Renamed Game";
                if (!window.SavePackaging(check->Output, check->Executable, {}) || !window.PackageProject())
                { finish(false, "could not remove logo and change executable name"); return; }
                ++check->Phase; break;
            case 7:
                if (!window.LastPackagingSucceeded() || read(check->Project) != check->Metadata ||
                    read(check->Output + "/Config/Config.ini") != check->PreservedConfig ||
                    read(check->Output + "/Cache/keep.txt") != "persistent cache" ||
                    !QFileInfo::exists(check->Output + "/Renamed Game.exe") ||
                    !QFileInfo::exists(check->Output + "/Runtime/Renamed Game.exe") ||
                    !QFileInfo::exists(check->Output + "/Data/Renamed Game.gpak") ||
                    QFileInfo::exists(check->Output + "/TriangleGame.exe") ||
                    ExtractIconExW(reinterpret_cast<LPCWSTR>((check->Output + "/Renamed Game.exe").utf16()),
                        -1, nullptr, nullptr, 0) != 0)
                { finish(false, "renaming or clearing cached logo failed"); return; }
                check->Manifest = read(check->Output + "/Metadata/.gengine-output.json");
                if (!window.OpenProject(check->Project) || !window.PackageProject())
                { finish(false, "empty logo preference did not survive reopen"); return; }
                ++check->Phase; break;
            case 8:
                if (!window.LastPackagingSucceeded() ||
                    read(check->Output + "/Metadata/.gengine-output.json") != check->Manifest ||
                    !window.findChild<QPlainTextEdit*>("ConsoleOutput")->toPlainText().contains("\"copyFiles\":0"))
                { finish(false, "repeat after removing logo failed"); return; }
                window.CloseProject();
                if (window.findChild<QAction*>("PackageProjectAction")->isEnabled()) { finish(false, "closed project can still package"); return; }
                finish(true,"metadata-only .gproject, output/name/logo dialog, invalid-input rejection, real automatic Release build and .gpak, custom manifest and icon, incremental repeat, build failure preservation, cancel/retry, local preferences and project close");
                break;
            }
        });
        timer->start();
    }
}
