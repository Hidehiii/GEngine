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
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace GEngineEditor
{
    namespace
    {
        void Require(bool value, const char* message)
        { if (!value) throw std::runtime_error(message); }
        QByteArray Read(const QString& path)
        { QFile file(path); Require(file.open(QIODevice::ReadOnly), "read fixture"); return file.readAll(); }
        void Write(const QString& path, const QByteArray& bytes)
        {
            Require(QDir().mkpath(QFileInfo(path).absolutePath()), "fixture directory");
            QFile file(path); Require(file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size(), "write fixture");
        }
    }

    void RunProjectSettingsCheck(MainWindow& window)
    {
        struct Check
        {
            QTemporaryDir Directory;
            QElapsedTimer Clock;
            QString A, B, Alternate;
            int Phase = 0;
            bool StartupChecked = false;
            QString DialogError;
        };
        auto check = std::make_shared<Check>(); check->Clock.start();
        auto* timer = new QTimer(&window); timer->setInterval(20);
        QObject::connect(timer, &QTimer::timeout, &window, [&window, timer, check]
        {
            try
            {
                Require(check->Clock.elapsed() < 55000, "project settings check timeout");
                auto& worker = window.Worker();
                if (check->Phase && worker.GetState() == WorkerSupervisor::State::Disconnected)
                    throw std::runtime_error("scene Worker disconnected");
                switch (check->Phase)
                {
                case 0:
                {
                    const auto args = QCoreApplication::arguments();
                    const auto projectIndex = args.indexOf("--project");
                    const auto startup = projectIndex >= 0 ? args.value(projectIndex + 1) :
                        args.value(1).endsWith(".gproject", Qt::CaseInsensitive) ? args[1] : QString{};
                    if (!startup.isEmpty() && !check->StartupChecked)
                    {
                        if (worker.GetState() != WorkerSupervisor::State::Ready || !window.HasImage()) break;
                        ProjectFile project; QString error;
                        Require(ProjectFile::Load(startup, project, error) &&
                            window.ActiveScenePath() == project.ResolveDefaultScene(error), "CLI startup did not load project default scene");
                        if (args.contains("--engine-check")) Require(worker.IsEngine(), "CLI startup used diagnostic worker");
                        check->StartupChecked = true;
                    }
                    timer->stop();
                    Require(check->Directory.isValid(), "temporary project unavailable");
                    const auto directory = check->Directory.path();
                    check->A = directory + "/A/A.gproject";
                    check->B = directory + "/B/B.gproject";
                    check->Alternate = directory + "/A/Assets/Scenes/Alternate.GEScene";
                    Write(directory + "/A/Assets/Scenes/Main.GEScene", "Scene: Project A\nGameObjects: []\n");
                    Write(check->Alternate, "Scene: Alternate\nGameObjects: []\n");
                    Write(directory + "/B/Assets/Scenes/Main.GEScene", "Scene: Project B\nGameObjects: []\n");
                    QString error;
                    ProjectFile first{check->A, "A", "Assets/Scenes/Main.GEScene"};
                    ProjectFile second{check->B, "B", "Assets/Scenes/Main.GEScene"};
                    Require(first.Save(error) && second.Save(error), "save project settings fixtures");
                    ProjectFile loaded;
                    Require(ProjectFile::Load(check->A, loaded, error) && loaded.DefaultScene == first.DefaultScene,
                        "settings round trip");
                    const auto invalid = directory + "/Invalid.gproject";
                    for (const auto& scene : {QString("../outside.GEScene"), QString("C:/outside.GEScene"), QString("Assets/file.txt")})
                    {
                        auto object = QJsonDocument::fromJson(Read(check->A)).object();
                        object["settings"] = QJsonObject{{"defaultScene", scene}};
                        Write(invalid, QJsonDocument(object).toJson());
                        Require(!ProjectFile::Load(invalid, loaded, error) && loaded.DefaultScene == first.DefaultScene,
                            "invalid scene path changed parsed state");
                    }
                    Write(invalid, "{\"schemaVersion\":1,\"name\":\"Bad\",\"settings\":{\"defaultScene\":12}}");
                    Require(!ProjectFile::Load(invalid, loaded, error), "non-string setting accepted");
                    Write(invalid, "{\"schemaVersion\":1,\"name\":\"Bad\",\"settings\":{\"unknown\":\"x\"}}");
                    Require(!ProjectFile::Load(invalid, loaded, error), "unknown setting accepted");
                    Require(window.OpenProject(check->A) && window.ActiveScenePath() == first.ResolveDefaultScene(error),
                        "open project did not select its default scene");
                    const auto before = Read(check->A);
                    Require(!window.SaveProjectSettings("../outside.GEScene") && Read(check->A) == before,
                        "invalid settings replaced project");
                    const auto backup = check->A + ".backup";
                    Require(QFile::rename(check->A, backup) && QDir().mkdir(check->A), "failed-save setup");
                    const bool saved = window.SaveProjectSettings("Assets/Scenes/Alternate.GEScene");
                    Require(QDir().rmdir(check->A) && QFile::rename(backup, check->A), "failed-save fixture restore");
                    Require(!saved && window.DefaultScene() == first.DefaultScene && Read(check->A) == before,
                        "failed save changed project/settings");
                    auto* action = window.findChild<QAction*>("ProjectSettingsAction");
                    Require(action && action->isEnabled(), "project settings action unavailable");
                    auto show = [&window, check, action](bool save)
                    {
                        QTimer::singleShot(0, &window, [&window, check, save]
                        {
                            auto* dialog = window.findChild<QDialog*>("ProjectSettingsDialog");
                            if (!dialog) { check->DialogError = "settings dialog unavailable"; QApplication::exit(1); return; }
                            auto* scene = dialog->findChild<QLineEdit*>("ProjectDefaultScene");
                            auto* buttons = dialog->findChild<QDialogButtonBox*>();
                            scene->setText(save ? "Assets/Scenes/Alternate.GEScene" : "../invalid.GEScene");
                            const auto args = QCoreApplication::arguments();
                            const auto image = args.indexOf("--project-settings-image");
                            if (save && image >= 0 && image + 1 < args.size() && !dialog->grab().save(args[image + 1]))
                                check->DialogError = "could not capture project settings dialog";
                            buttons->button(QDialogButtonBox::Save)->click();
                            if (!save)
                            {
                                if (dialog->findChild<QLabel*>("ProjectSettingsError")->text().isEmpty())
                                    check->DialogError = "invalid settings did not report error";
                                buttons->button(QDialogButtonBox::Cancel)->click();
                            }
                        });
                        action->trigger();
                        Require(check->DialogError.isEmpty(), "project settings dialog validation failed");
                    };
                    show(false);
                    Require(Read(check->A) == before && window.DefaultScene() == first.DefaultScene, "cancel changed settings");
                    show(true);
                    Require(window.DefaultScene() == "Assets/Scenes/Alternate.GEScene" &&
                        ProjectFile::Load(check->A, loaded, error) && loaded.DefaultScene == window.DefaultScene(), "dialog save not persisted");
                    ++check->Phase; timer->start(); break;
                }
                case 1:
                    if (worker.GetState() != WorkerSupervisor::State::Ready || window.FrameCount() < (worker.IsEngine() ? 3u : 1u)) break;
                    if (QCoreApplication::arguments().contains("--engine-check"))
                        Require(window.findChild<QPlainTextEdit*>("ConsoleOutput")->toPlainText().contains("Editor scene loaded: Alternate"),
                            "Worker did not load persisted default scene");
                    Require(window.OpenProject(check->B) && window.ActiveScenePath() != check->Alternate, "project switch retained old scene");
                    ++check->Phase; break;
                case 2:
                    if (worker.GetState() != WorkerSupervisor::State::Ready || window.FrameCount() < (worker.IsEngine() ? 3u : 1u)) break;
                    if (QCoreApplication::arguments().contains("--engine-check"))
                        Require(window.findChild<QPlainTextEdit*>("ConsoleOutput")->toPlainText().contains("Editor scene loaded: Project B"),
                            "Worker did not switch project scene");
                    Write(check->Directory.filePath("Legacy.gproject"), "{\"schemaVersion\":1,\"name\":\"Legacy\"}");
                    Require(window.OpenProject(check->Directory.filePath("Legacy.gproject")), "legacy project rejected");
                    ++check->Phase; break;
                case 3:
                {
                    if (worker.Running()) break;
                    Require(window.ActiveScenePath().isEmpty() && !window.HasImage(), "legacy project kept old scene image");
                    window.StartWorker(); Require(!worker.Running(), "unset default silently started sample");
                    ProjectFile missing{check->Directory.filePath("Missing.gproject"), "Missing", "Assets/Missing.GEScene"};
                    QString error; Require(missing.Save(error) && window.OpenProject(missing.Path), "missing scene prevented project repair");
                    Require(window.ActiveScenePath().isEmpty() && !window.SaveProjectSettings(missing.DefaultScene), "missing default accepted for saving");
                    Require(window.SaveProjectSettings({}), "clear default scene setting");
                    Require(window.NewProject(check->Directory.filePath("New/New.gproject")), "new project creation");
                    Require(window.DefaultScene() == "Assets/Scenes/Default.GEScene" && QFileInfo::exists(window.ActiveScenePath()),
                        "new project lacks configured default scene");
                    ++check->Phase; break;
                }
                case 4:
                    if (worker.GetState() != WorkerSupervisor::State::Ready || !window.HasImage()) break;
                    window.CloseProject(); ++check->Phase; break;
                case 5:
                    if (worker.Running()) break;
                    Require(window.ActiveScenePath().isEmpty() && !window.HasImage(), "closed project retained scene");
                    timer->stop(); std::cout << "PASS: project settings save/cancel/failure, paths, defaults, switching, legacy/missing and new project\n";
                    QApplication::exit(0); break;
                }
            }
            catch (const std::exception& error)
            {
                timer->stop();
                std::cerr << "FAIL: " << error.what() << '\n'
                    << window.findChild<QPlainTextEdit*>("ConsoleOutput")->toPlainText().toStdString() << '\n';
                QApplication::exit(1);
            }
        });
        timer->start();
    }
}
