#pragma once
#include <QString>

namespace GEngineEditor
{
    struct ProjectFile
    {
        QString Path;
        QString Name;
        static bool Load(const QString& path, ProjectFile& result, QString& error);
        bool Save(QString& error) const;
        QString Root() const;
        QString Resolve(const QString& path) const;
    };
}
