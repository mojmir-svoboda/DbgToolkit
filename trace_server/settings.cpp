#include "settings.h"
#include "ui_settings.h"
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QStandardItem>
#include "types.h"
#include "utils.h"
#include "mainwindow.h"
#include "connection.h"
#include <tlv_parser/tlv_parser.h>

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

void MainWindow::onClickedAtSettingColumnSetup (QModelIndex idx)
{ }
void MainWindow::onClickedAtSettingColumnSizes (QModelIndex idx)
{ }

void MainWindow::onClickedAtSettingColumnAlign (QModelIndex idx)
{
	QString const txt = qVariantValue<QString>(ui_settings->listViewColumnAlign->model()->data(idx));
	E_Align const curr = stringToAlign(txt.toStdString().c_str()[0]);
	size_t i = (curr + 1) % e_max_align_enum_value;
	E_Align const act = static_cast<E_Align>(i);
	ui_settings->listViewColumnAlign->model()->setData(idx, QString(alignToString(act)));
}
void MainWindow::onClickedAtSettingColumnElide (QModelIndex idx)
{
	QString const txt = qVariantValue<QString>(ui_settings->listViewColumnElide->model()->data(idx));
	E_Elide const curr = stringToElide(txt.toStdString().c_str()[0]);
	size_t i = (curr + 1) % e_max_elide_enum_value;
	E_Elide const act = static_cast<E_Elide>(i);
	ui_settings->listViewColumnElide->model()->setData(idx, QString(elideToString(act)));
}

void onEditingFinishedOfColumnSizes (QModelIndex idx);

template<class C>
void clearListView (C * v)
{
	if (v && v->model())
	{
		static_cast<QStandardItemModel *>(v->model())->clear();
	}
}

class MyListModel : public QStandardItemModel
{
	QList<QAbstractItemModel *> m_observers;

public:
	MyListModel (QObject * parent = 0 );

	void addObserver (QAbstractItemModel * o) { m_observers.append(o); }

	Qt::ItemFlags flags (QModelIndex const & index) const
	{
		if (index.isValid())
			return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
		return Qt::ItemIsDropEnabled;
	}

	bool add (QString const & tlvname, int row, bool checked)
	{
		//insertRow(row, addRow(tlvname, checked));
		//beginInsertRows(QModelIndex(), row, row);
		//insertRow(m_data.count());
		//m_data.insert(row, tlvname);
		//endInsertRows();
		//emit layoutChanged();
		return true;
	}

	/*QVariant data (QModelIndex const & index, int role = Qt::DisplayRole) const
	{
		if (!index.isValid()) return QVariant();
		return QVariant(m_data[index.row()]);
	}
	int rowCount (QModelIndex const & parent) const
	{
		if (parent.isValid()) return 0;
		else return m_data.size();
	}
	int columnCount (QModelIndex const & parent) const { return 1; }*/


	bool dropMimeData (QMimeData const * mm, Qt::DropAction action, int row, int column, QModelIndex const & parent)
	{
		qDebug("%s row=%i", __FUNCTION__, row);
		if (!mm->hasFormat("text/x-tlv-name"))
			return false;

		if (action == Qt::IgnoreAction)
			return true;

		if (column > 0)
			return false;

		int endRow = -1;
		if (!parent.isValid()) {
			if (row < 0)
				endRow = rowCount();
			else
				endRow = qMin(row, rowCount());
		} else
			endRow = parent.row();

		QByteArray encodedData = mm->data("text/x-tlv-name");
		QDataStream stream(&encodedData, QIODevice::ReadOnly);

		while (!stream.atEnd())
		{
			QString tlvname;
			int orig_row = -1;
			stream >> tlvname >> orig_row;

			qDebug("drop: %s, %i -> %i", tlvname.toStdString().c_str(), orig_row, endRow);
			insertRow(endRow, addRow(tlvname, true));

			for (size_t i = 0, ie = m_observers.size(); i < ie; ++i)
			{
				QString txt = qVariantValue<QString>(m_observers.at(i)->data(m_observers.at(i)->index(orig_row, 0, QModelIndex())));
				m_observers.at(i)->removeRows(orig_row, 1);
				static_cast<QStandardItemModel *>(m_observers.at(i))->insertRow(endRow - 1, addUncheckableRow(txt));
			}
			//beginInsertRows(QModelIndex(), endRow, endRow);
			//m_data.insert(endRow, tlvname);
			//endInsertRows();

			++endRow;
		}
		return true;
	}

	QMimeData * mimeData (QModelIndexList const & indexes) const
	{
		QMimeData * mimeData = new QMimeData();
		QByteArray encodedData;
		QDataStream stream(&encodedData, QIODevice::WriteOnly);
		foreach (QModelIndex index, indexes)
		{
			if (index.isValid())
			{
				QString tlvname = qVariantValue<QString>(data(index, Qt::DisplayRole));
				stream << tlvname << index.row();
				//qDebug("drag: %s, %i", tlvname.toStdString().c_str(), index.row());
			}
		}

		mimeData->setData("text/x-tlv-name", encodedData);
		return mimeData;
	}

	QStringList mimeTypes () const
	{
		QStringList types;
		types << "text/x-tlv-name";
		return types;
	}

	Qt::DropActions supportedDropActions () const
	{
		return Qt::MoveAction;
	}
};

MyListModel::MyListModel (QObject * parent) : QStandardItemModel(parent) { }

void MainWindow::onSettingsAppSelected (int idx)
{
	clearListView(ui_settings->listViewColumnSetup);
	clearListView(ui_settings->listViewColumnSizes);
	clearListView(ui_settings->listViewColumnAlign);
	clearListView(ui_settings->listViewColumnElide);

	QStandardItem * cs_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnSetup->model())->invisibleRootItem();
	for (int i = 0, ie = m_columns_setup[idx].size(); i < ie; ++i)
	{
		QList<QStandardItem *> row_items = addRow(m_columns_setup.at(idx).at(i), true);
		cs_root->appendRow(row_items);
	}

	//connect(ui_settings->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	//connect(ui_settings->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	connect(ui_settings->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	connect(ui_settings->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));

	QStandardItem * csz_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnSizes->model())->invisibleRootItem();
	for (int i = 0, ie = m_columns_sizes[idx].size(); i < ie; ++i)
	{
		QList<QStandardItem *> row_items = addUncheckableRow(tr("%1").arg(m_columns_sizes.at(idx).at(i)));
		csz_root->appendRow(row_items);
	}

	QStandardItem * cal_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnAlign->model())->invisibleRootItem();
	for (int i = 0, ie = m_columns_align[idx].size(); i < ie; ++i)
	{
		QList<QStandardItem *> row_items = addUncheckableRow(tr("%1").arg(m_columns_align.at(idx).at(i)));
		cal_root->appendRow(row_items);
	}

	QStandardItem * cel_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnElide->model())->invisibleRootItem();
	for (int i = 0, ie = m_columns_elide[idx].size(); i < ie; ++i)
	{
		QList<QStandardItem *> row_items = addUncheckableRow(tr("%1").arg(m_columns_elide.at(idx).at(i)));
		cel_root->appendRow(row_items);
	}


	/*size_t const n = tlv::get_tag_count();
	for (size_t i = tlv::tag_time; i < n; ++i)
	{
		char const * name = tlv::get_tag_name(i);
		if (name)
		{
			QList<QStandardItem *> row_items = addRow(QString::fromAscii(name), true);
			cs_root->appendRow(row_items);
			
			//tags += QString::fromStdString(name) + "	";
		}
	}*/
}




void MainWindow::onSetup ()
{
	ui_settings->comboBoxApp->clear();
	for (int a = 0, ae = m_app_names.size(); a < ae; ++a)
		ui_settings->comboBoxApp->addItem(m_app_names.at(a));

	int idx = 0;
	Connection * conn = m_server->findCurrentConnection();
	if (conn)
	{
		idx = conn->sessionState().m_app_idx;
	}

	connect(ui_settings->comboBoxApp, SIGNAL(activated(int)), this, SLOT(onSettingsAppSelected(int)));

	MyListModel * model = new MyListModel(this);
	ui_settings->listViewColumnSetup->setModel(model);
	ui_settings->listViewColumnSetup->model()->setSupportedDragActions(Qt::MoveAction);
	ui_settings->listViewColumnSizes->setModel(new QStandardItemModel(this));
	ui_settings->listViewColumnAlign->setModel(new QStandardItemModel(this));
	ui_settings->listViewColumnElide->setModel(new QStandardItemModel(this));
	ui_settings->listViewColumnSetup->setDropIndicatorShown(true);
	ui_settings->listViewColumnSetup->setMovement(QListView::Snap);
	ui_settings->listViewColumnSetup->setDragDropMode(QAbstractItemView::InternalMove);
	model->addObserver(ui_settings->listViewColumnSizes->model());
	model->addObserver(ui_settings->listViewColumnAlign->model());
	model->addObserver(ui_settings->listViewColumnElide->model());
	ui_settings->listViewColumnSetup->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui_settings->listViewColumnAlign->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui_settings->listViewColumnElide->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//ui_settings->listViewColumnSizes->setItemDelegate(new SettingsEditDelegate());

	// if sz > 0
	onSettingsAppSelected(idx);

	if (m_settings_dialog->exec() == QDialog::Accepted)
	{
/*		for (int i = 0, ie = model.m_app_names.size(); i < ie; ++i)
		{
			int const idx = findAppName(m_app_names[i]);
			qDebug("app=%s", m_app_names.at(i).toStdString().c_str());
			m_columns_setup[idx].clear();
			m_columns_sizes[idx].clear();
			m_columns_align[idx].clear();
			m_columns_elide[idx].clear();
			if (idx >= 0)
			{
				size_t j = 0;
				for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it, ++j)
				{
					qDebug("  token=%s", it->c_str());
					m_columns_setup[idx].append(QString::fromStdString(*it));
					m_columns_sizes[idx].append(127);
				}
			}
		}

		if (Connection * conn = m_server->findCurrentConnection())
			conn->onApplyColumnSetup();
*/
	}
}







/// old stuff

/*SettingsModelView::SettingsModelView (QList<QString> const & app_names, QList<columns_setup_t> const & col_setup)
	: QAbstractTableModel(0)
	, m_app_names(app_names)
	, m_rows()
{
	for (size_t i = 0, ie = app_names.size(); i < ie; ++i) {
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
*/


