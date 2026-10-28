#include "ProjectFile.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace GEngineEditor
{
    QString ProjectFile::Root() const { return QFileInfo(Path).absolutePath(); }
    QString ProjectFile::Resolve(const QString& path) const
    { return path.isEmpty() ? QString{} : QDir::cleanPath(QDir(Root()).absoluteFilePath(path)); }

    bool ProjectFile::ValidateSettings(QString& error) const
    {
        if (DefaultScene.isEmpty()) { error.clear(); return true; }
        const auto path = QDir::cleanPath(QDir::fromNativeSeparators(DefaultScene));
        if (QDir::isAbsolutePath(path) || path.contains(':') || path.contains(QChar::Null) ||
            path == ".." || path.startsWith("../") ||
            QFileInfo(path).suffix().compare("GEScene", Qt::CaseInsensitive) != 0)
        { error = "Default scene must be a project-relative .GEScene path inside this project."; return false; }
        error.clear(); return true;
    }

    QString ProjectFile::ResolveDefaultScene(QString& error) const
    {
        if (!ValidateSettings(error)) return {};
        if (DefaultScene.isEmpty()) { error = "No default scene is configured. Choose one in Project settings."; return {}; }
        const QFileInfo scene(Resolve(DefaultScene));
        const auto root = QFileInfo(Root()).canonicalFilePath();
        const auto canonical = scene.canonicalFilePath();
        if (!scene.isFile() || canonical.isEmpty())
        { error = "Default scene is missing: " + DefaultScene; return {}; }
        const auto prefix = root.endsWith('/') ? root : root + '/';
        if (root.isEmpty() || !canonical.startsWith(prefix, Qt::CaseInsensitive))
        { error = "Default scene resolves outside this project: " + DefaultScene; return {}; }
        error.clear(); return canonical;
    }

    bool ProjectFile::Load(const QString& path, ProjectFile& result, QString& error)
    {
        const QFileInfo info(path);
        if (info.suffix().compare("gproject", Qt::CaseInsensitive) != 0 || info.isSymLink())
        { error = "Choose a .gproject file."; return false; }
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly) || file.size() > 1024 * 1024)
        { error = "Project file is unreadable or exceeds 1 MiB."; return false; }
        QJsonParseError parse;
        const auto document = QJsonDocument::fromJson(file.readAll(), &parse);
        const auto object = document.object();
        if (parse.error != QJsonParseError::NoError || !document.isObject() ||
            object.value("schemaVersion").toInt(-1) != 1 || !object.value("name").isString() ||
            object.value("name").toString().trimmed().isEmpty() || object.contains("packaging"))
        { error = "Invalid project format/version."; return false; }
        for (auto it = object.begin(); it != object.end(); ++it)
            if (it.key() != "schemaVersion" && it.key() != "name" && it.key() != "settings")
            { error = "Unknown project field: " + it.key(); return false; }
        ProjectFile candidate;
        candidate.Path = info.canonicalFilePath();
        candidate.Name = object.value("name").toString();
        if (object.contains("settings"))
        {
            if (!object.value("settings").isObject())
            { error = "Project settings must be an object."; return false; }
            const auto settings = object.value("settings").toObject();
            for (auto it = settings.begin(); it != settings.end(); ++it)
                if (it.key() != "defaultScene" || !it.value().isString())
                { error = "Invalid project setting: " + it.key(); return false; }
            candidate.DefaultScene = settings.value("defaultScene").toString();
        }
        if (!candidate.ValidateSettings(error)) return false;
        if (!candidate.DefaultScene.isEmpty()) candidate.DefaultScene = QDir::cleanPath(QDir::fromNativeSeparators(candidate.DefaultScene));
        result = candidate; error.clear(); return true;
    }
    bool ProjectFile::Save(QString& error) const
    {
        if (QFileInfo(Path).suffix().compare("gproject", Qt::CaseInsensitive) != 0 ||
            QFileInfo(Path).isSymLink() || Name.trimmed().isEmpty())
        { error = "Project name/path is invalid."; return false; }
        if (!ValidateSettings(error)) return false;
        const auto scene = DefaultScene.isEmpty() ? QString{} : QDir::cleanPath(QDir::fromNativeSeparators(DefaultScene));
        const auto bytes = QJsonDocument(QJsonObject{{"schemaVersion", 1}, {"name", Name},
            {"settings", QJsonObject{{"defaultScene", scene}}}}).toJson();
        QSaveFile file(Path);
        file.setDirectWriteFallback(false);
        if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        { error = "Could not save project: " + file.errorString(); return false; }
        error.clear(); return true;
    }
}
