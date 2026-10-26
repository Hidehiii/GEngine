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
        if (parse.error != QJsonParseError::NoError || !document.isObject() || object.size() != 2 ||
            object.value("schemaVersion").toInt(-1) != 1 || !object.value("name").isString() ||
            object.value("name").toString().trimmed().isEmpty() || object.contains("packaging"))
        { error = "Invalid project format/version. Project files contain project metadata only."; return false; }
        ProjectFile candidate;
        candidate.Path = info.canonicalFilePath();
        candidate.Name = object.value("name").toString();
        result = candidate; error.clear(); return true;
    }
    bool ProjectFile::Save(QString& error) const
    {
        if (QFileInfo(Path).suffix().compare("gproject", Qt::CaseInsensitive) != 0 ||
            QFileInfo(Path).isSymLink() || Name.trimmed().isEmpty())
        { error = "Project name/path is invalid."; return false; }
        const auto bytes = QJsonDocument(QJsonObject{{"schemaVersion", 1}, {"name", Name}}).toJson();
        QSaveFile file(Path);
        if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        { error = "Could not save project: " + file.errorString(); return false; }
        error.clear(); return true;
    }
}
