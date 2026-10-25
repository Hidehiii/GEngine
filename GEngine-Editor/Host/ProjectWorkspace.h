#pragma once
#include "Core/ProjectContext.h"
#include <QWidget>
#include <QTimer>
#include <functional>

class QFileSystemModel;
class QTreeView;
class QLabel;
class QAbstractItemModel;

namespace GEngineEditor
{
    class ProjectWorkspace final : public QWidget
    {
    public:
        explicit ProjectWorkspace(QWidget* parent = nullptr);
        bool Open(const QString& directory);
        void Close();
        bool Select(const QString& path);
        void ValidateSelection();
        QString Root() const;
        QString SelectedPath() const { return m_Selected; }
        std::uint64_t Generation() const { return m_Context.Generation(); }
        std::function<void(const QString&)> OnInspector;
        std::function<void(const QString&)> OnLog;
    private:
        void ClearSelection();
        void SetTreeModel(QAbstractItemModel* model);
        ProjectContext m_Context;
        QFileSystemModel* m_Model = nullptr;
        QTreeView* m_Tree = nullptr;
        QLabel* m_Label = nullptr;
        QTimer m_Validation;
        QString m_Selected;
        std::uint64_t m_SelectionGeneration = 0;
    };
}
