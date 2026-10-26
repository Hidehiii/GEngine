#include "ProjectWorkspace.h"
#include <QFileSystemModel>
#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>
#include <QDir>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QDateTime>
#include <QHeaderView>

namespace GEngineEditor
{
    static std::filesystem::path NativePath(const QString& path)
    {
#ifdef _WIN32
        return std::filesystem::path(path.toStdWString());
#else
        return std::filesystem::path(path.toStdString());
#endif
    }
    ProjectWorkspace::ProjectWorkspace(QWidget* parent) : QWidget(parent)
    {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(4, 4, 4, 4);
        m_Label = new QLabel("No project open", this);
        m_Label->setWordWrap(true);
        m_Label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        layout->addWidget(m_Label);
        m_Tree = new QTreeView(this);
        m_Tree->setObjectName("AssetsTree");
        m_Tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_Tree->setDragEnabled(false);
        m_Tree->setContextMenuPolicy(Qt::NoContextMenu);
        layout->addWidget(m_Tree);
        m_Validation.setInterval(250);
        connect(&m_Validation, &QTimer::timeout, this, [this] { ValidateSelection(); });
        m_Validation.start();
    }
    QString ProjectWorkspace::Root() const
    {
#ifdef _WIN32
        return QString::fromStdWString(m_Context.Root().native());
#else
        return QString::fromStdString(m_Context.Root().native());
#endif
    }
    QString ProjectWorkspace::SelectedPath() const
    {
        if (m_Selection.Empty() || m_Context.Root().empty()) return {};
        return QDir(Root()).filePath(QString::fromStdString(m_Selection.Id()));
    }
    bool ProjectWorkspace::Open(const QString& directory)
    {
        // Validate before replacing either the authoritative root or its UI projection.
        if (!QFileInfo(directory).isReadable() || !m_Context.Open(NativePath(directory))) return false;
        auto* oldModel = m_Model;
        m_Model = new QFileSystemModel(this);
        m_Model->setReadOnly(true);
        m_Model->setOption(QFileSystemModel::DontResolveSymlinks, true);
        m_Model->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        const auto root = m_Model->setRootPath(Root());
        SetTreeModel(m_Model);
        delete oldModel;
        m_Tree->setRootIndex(root);
        for (int column = 1; column < m_Model->columnCount(); ++column) m_Tree->hideColumn(column);
        m_Tree->setHeaderHidden(true);
        const auto name = QFileInfo(Root()).fileName();
        m_Label->setText(name.isEmpty() ? QDir::toNativeSeparators(Root()) : name);
        m_Label->setToolTip(QDir::toNativeSeparators(Root()));
        ClearSelection();
        connect(m_Tree->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex& index)
            {
                if (index.isValid()) Select(m_Model->filePath(index));
                else ValidateSelection();
            });
        const auto generation = m_Context.Generation();
        connect(m_Model, &QFileSystemModel::directoryLoaded, this, [this, generation](const QString&)
        {
            const auto selected = SelectedPath();
            if (generation != m_Context.Generation() || !m_Model || selected.isEmpty()) return;
            ValidateSelection();
            if (!SelectedPath().isEmpty()) m_Tree->setCurrentIndex(m_Model->index(SelectedPath()));
        });
        return true;
    }
    void ProjectWorkspace::Close()
    {
        m_Context.Close();
        if (m_Model)
        {
            SetTreeModel(nullptr);
            delete m_Model; m_Model = nullptr;
        }
        m_Label->setText("No project open");
        m_Label->setToolTip({});
        ClearSelection();
    }
    void ProjectWorkspace::ClearSelection()
    {
        m_Selection.Clear();
        if (m_Tree->selectionModel()) m_Tree->selectionModel()->clear();
        if (OnInspector) OnInspector("Select a file or folder in Assets.\nRead-only project browsing; scene editing is not connected yet.");
    }
    void ProjectWorkspace::SetTreeModel(QAbstractItemModel* model)
    {
        m_Tree->setModel(model);
        // This private tree/header share selection; no other views borrow these models.
        // Qt retains replaced selection models, including a transient header selection.
        for (auto* selection : m_Tree->findChildren<QItemSelectionModel*>())
            if (selection != m_Tree->selectionModel() && selection != m_Tree->header()->selectionModel())
                delete selection;
    }
    bool ProjectWorkspace::Select(const QString& path)
    {
        const QFileInfo file(path);
        const auto selectedPath = file.canonicalFilePath();
        if (file.isSymLink() || selectedPath.isEmpty() || !m_Context.Contains(NativePath(selectedPath)))
        { ClearSelection(); return false; }
        const auto relative = QDir(Root()).relativeFilePath(selectedPath);
        if (!m_Selection.Select(m_Context.Generation(), relative.toStdString()))
        { ClearSelection(); return false; }
        if (OnInspector) OnInspector(QString("%1\n\nPath: %2\nType: %3\nSize: %4\nModified: %5\n\nRead-only metadata; file contents are not loaded or executed.")
            .arg(file.fileName(), relative, file.isDir() ? "Folder" : file.suffix().isEmpty() ? "File" : file.suffix(),
                file.isDir() ? "-" : QString::number(file.size()) + " bytes", file.lastModified().toString(Qt::ISODate)));
        return true;
    }
    void ProjectWorkspace::ValidateSelection()
    {
        if (!Root().isEmpty() && !QFileInfo(Root()).isDir())
        {
            Close();
            if (OnLog) OnLog("Project directory is unavailable; project closed. Draft and worker are preserved.");
            return;
        }
        const auto selected = SelectedPath();
        if (!selected.isEmpty())
        {
            const QFileInfo file(selected);
            const bool valid = !file.isSymLink() && file.exists() && m_Context.Contains(NativePath(file.canonicalFilePath()));
            if (!m_Selection.Validate(m_Context.Generation(), valid)) ClearSelection();
            else Select(selected);
        }
    }
}
