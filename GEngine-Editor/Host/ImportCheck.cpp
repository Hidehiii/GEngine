#include "MainWindow.h"
#include <QAction>
#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTimer>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace GEngineEditor
{
    void RunImportCheck(MainWindow& window)
    {
        struct Check
        {
            QTemporaryDir Directory;
            QElapsedTimer Elapsed;
            int Step = 0;
            QString Project, Source, Destination;
        };
        auto check = std::make_shared<Check>();
        check->Elapsed.start();
        auto* timer = new QTimer(&window);
        timer->setInterval(25);
        QObject::connect(timer, &QTimer::timeout, &window, [check, timer, &window]
        {
            auto require = [](bool value, const char* message)
            { if (!value) throw std::runtime_error(message); };
            auto write = [&](const QString& path, const QByteArray& bytes)
            {
                QFile file(path);
                require(file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size(), "fixture write");
            };
            try
            {
                require(check->Elapsed.elapsed() < 45000, "import test timeout");
                if (window.ImportRunning()) return;
                if (check->Step == 0)
                {
                    require(check->Directory.isValid(), "temporary directory");
                    check->Project = check->Directory.filePath("Game.gproject");
                    check->Source = check->Directory.filePath("source.glsl");
                    check->Destination = check->Directory.filePath("Assets/Imported");
                    write(check->Source, "void main() {}\n");
                    require(!window.ImportResource(check->Source, check->Destination), "no project rejection");
                    ProjectFile project{check->Project, "Import check", {}};
                    QString error;
                    require(project.Save(error) && window.OpenProject(check->Project), "open project");
                    require(!window.ImportResource(check->Source, check->Directory.path()), "outside Assets rejection");
                    require(window.ImportResource(check->Source, check->Destination), "start import");
                    require(window.ImportRunning(), "asynchronous operation");
                    require(!window.OpenProject(check->Project), "project switch blocked");
                    require(!window.findChild<QAction*>("ImportResourceAction")->isEnabled(), "import disabled while busy");
                    ++check->Step;
                }
                else if (check->Step == 1)
                {
                    require(window.LastImportSucceeded(), "text import success");
                    const auto target = check->Destination + "/source.gasset";
                    require(QFileInfo::exists(target) && QFileInfo::exists(check->Source), "source retained and asset created");
                    QFile textAsset(target);
                    require(textAsset.open(QIODevice::ReadOnly), "read imported text");
                    const auto textBytes = textAsset.readAll();
                    require(textBytes.contains("type: shader\nformat: glsl\n") &&
                        textBytes.contains("void main() {}\\u000a"), "shader envelope contents");
                    require(window.Workspace().SelectedPath() == target, "import selection");
                    require(!window.ImportResource(check->Source, check->Destination), "duplicate rejection");
                    require(window.findChild<QAction*>("ImportResourceAction")->isEnabled(), "action restored");
                    check->Source = check->Directory.filePath("binary.png");
                    write(check->Source, QByteArray::fromHex("000102ff"));
                    require(window.ImportResource(check->Source, check->Destination), "binary start");
                    ++check->Step;
                }
                else if (check->Step == 2)
                {
                    require(window.LastImportSucceeded(), "binary import success");
                    QFile binaryAsset(check->Destination + "/binary.gasset");
                    require(binaryAsset.open(QIODevice::ReadOnly), "read imported binary");
                    const auto binaryBytes = binaryAsset.readAll();
                    require(binaryBytes.contains("type: texture\nformat: png\n") &&
                        binaryBytes.contains("AAEC/w=="), "binary envelope contents");
                    check->Source = check->Directory.filePath("invalid.glsl");
                    write(check->Source, QByteArray::fromHex("ff"));
                    require(window.ImportResource(check->Source, check->Destination), "invalid UTF-8 starts worker");
                    ++check->Step;
                }
                else
                {
                    require(!window.LastImportSucceeded() && !window.LastError().isEmpty(), "worker failure displayed");
                    require(!QFileInfo::exists(check->Destination + "/invalid.gasset"), "failure leaves no target");
                    require(QFileInfo::exists(check->Source), "failure preserves source");
                    window.CloseProject();
                    require(!window.findChild<QAction*>("ImportResourceAction")->isEnabled(), "closed project disables import");
                    timer->stop();
                    std::cout << "Editor resource import checks passed\n";
                    QApplication::exit(0);
                }
            }
            catch (const std::exception& error)
            {
                timer->stop();
                std::cerr << "Editor import check: " << error.what() << "; " << window.LastError().toStdString() << '\n';
                QApplication::exit(1);
            }
        });
        timer->start();
    }
}
