#pragma once
#include <QString>

namespace GEngineEditor
{
    struct ProjectFile
    {
        QString Path;
        QString Name;
        QString DefaultScene;
        static bool Load(const QString& path, ProjectFile& result, QString& error);
        bool Save(QString& error) const;
        QString Root() const;
        QString Resolve(const QString& path) const;
        QString ResolveDefaultScene(QString& error) const;
        bool ValidateSettings(QString& error) const;
    };
}
