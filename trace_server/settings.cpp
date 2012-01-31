#include "settings.h"
#include <QLineEdit>

SettingsModelView::SettingsModelView (QList<QString> const & app_names, QList<MainWindow::columns_setup_t> const & col_setup)
	: QAbstractTableModel(0)
	, m_app_names(app_names)
	, m_rows()
{
	for (size_t i = 0, ie = app_names.size(); i < ie; ++i)
	{
		m_rows.push_back(QString());

		for (size_t j = 0, je = col_setup.at(i).size(); j < je; ++j)
			m_rows.back() += col_setup.at(i).at(j) + QString(", ");
	}
}

SettingsModelView::~SettingsModelView () { }

int SettingsModelView::rowCount (const QModelIndex &) const { return m_app_names.size(); }

int SettingsModelView::columnCount (const QModelIndex &) const { return 2; }

QVariant SettingsModelView::data (const QModelIndex & index, int role) const
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		QString str("");
		if (index.column() == 0)
			if (index.row() < m_app_names.size())
				return m_app_names[index.row()];
		if (index.column() == 1)
			if (index.row() < (int)m_rows.size())
				return m_rows[index.row()];
		return str;
	}
	return QVariant();
}

bool SettingsModelView::setData (QModelIndex const & index, QVariant const & value, int role)
{
	if (role == Qt::EditRole)
	{
		if (index.column() == 1)
		{
			m_rows[index.row()] = value.toString();
			return true;
		}
	}
	return false;
}

QVariant SettingsModelView::headerData (int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		if (section == 0)
			return QString("Application name");
		if (section == 1)	
			return QString("tags separated by ,");
	}
	return QVariant();
}

Qt::ItemFlags SettingsModelView::flags (QModelIndex const & index) const
{
	 return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

SettingsEditDelegate::SettingsEditDelegate(QObject *parent) : QItemDelegate(parent) {}

QWidget *SettingsEditDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &,
                                          const QModelIndex &index) const
{
    QLineEdit * editor = new QLineEdit(parent);
    connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
    return editor;
}

void SettingsEditDelegate::commitAndCloseEditor()
{
    QLineEdit * editor = qobject_cast<QLineEdit *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

void SettingsEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit * edit = qobject_cast<QLineEdit*>(editor);
    if (edit)
        edit->setText(index.model()->data(index, Qt::EditRole).toString());
}

void SettingsEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit * edit = qobject_cast<QLineEdit *>(editor);
    if (edit)
        model->setData(index, edit->text());
}


