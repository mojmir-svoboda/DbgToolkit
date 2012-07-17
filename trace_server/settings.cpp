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

void MainWindow::syncSettingsViews (QListView const * const invoker, QModelIndex const idx)
{
	QListView * const views[] = { ui_settings->listViewColumnSetup, ui_settings->listViewColumnSizes, ui_settings->listViewColumnAlign, ui_settings->listViewColumnElide };

	for (size_t i = 0; i < sizeof(views) / sizeof(*views); ++i)
	{
		if (views[i] != invoker)
		{
			views[i]->selectionModel()->clearSelection();
			QModelIndex const other_idx = views[i]->model()->index(idx.row(), idx.column(), QModelIndex());
			views[i]->selectionModel()->select(other_idx, QItemSelectionModel::Select);
		}
	}
}

void MainWindow::onClickedAtSettingColumnSetup (QModelIndex const idx)
{
	syncSettingsViews(ui_settings->listViewColumnSetup, idx);

	QStandardItem * const item = static_cast<QStandardItemModel *>(ui_settings->listViewColumnSetup->model())->itemFromIndex(idx);
	Qt::CheckState const curr = item->checkState();

	item->setCheckState(curr == Qt::Checked ? Qt::Unchecked : Qt::Checked);

	QModelIndex const size_idx = ui_settings->listViewColumnSizes->model()->index(idx.row(), idx.column(), QModelIndex());
	if (curr == Qt::Checked)
	{
		//ui_settings->listViewColumnSizes->model()->setData(size_idx, QString("0"));
	}
	else
	{
		// this does not work at all
		/*int app_idx = 0;
		Connection * conn = m_server->findCurrentConnection();
		if (conn)
			app_idx = conn->sessionState().m_app_idx;

		int size_val = 64;
		if (app_idx < m_columns_sizes.size())
		{
			if (size_idx.row() < m_columns_sizes[app_idx].size())
			{
				size_val = m_columns_sizes[app_idx].at(size_idx.row());
				ui_settings->listViewColumnSizes->model()->setData(size_idx, tr("%1").arg(size_val));
			}
		}*/
	}
}
void MainWindow::onClickedAtSettingColumnSizes (QModelIndex const idx)
{
	syncSettingsViews(ui_settings->listViewColumnSizes, idx);
}
void MainWindow::onClickedAtSettingColumnAlign (QModelIndex const idx)
{
	QString const txt = qVariantValue<QString>(ui_settings->listViewColumnAlign->model()->data(idx));
	E_Align const curr = stringToAlign(txt.toStdString().c_str()[0]);
	size_t i = (curr + 1) % e_max_align_enum_value;
	E_Align const act = static_cast<E_Align>(i);
	ui_settings->listViewColumnAlign->model()->setData(idx, QString(alignToString(act)));

	syncSettingsViews(ui_settings->listViewColumnAlign, idx);
}
void MainWindow::onClickedAtSettingColumnElide (QModelIndex const idx)
{
	QString const txt = qVariantValue<QString>(ui_settings->listViewColumnElide->model()->data(idx));
	E_Elide const curr = stringToElide(txt.toStdString().c_str()[0]);
	size_t i = (curr + 1) % e_max_elide_enum_value;
	E_Elide const act = static_cast<E_Elide>(i);
	ui_settings->listViewColumnElide->model()->setData(idx, QString(elideToString(act)));

	syncSettingsViews(ui_settings->listViewColumnElide, idx);
}

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

	bool dropMimeData (QMimeData const * mm, Qt::DropAction action, int row, int column, QModelIndex const & parent)
	{
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
			int check_state = Qt::Checked;
			stream >> tlvname >> orig_row >> check_state;

			//qDebug("drop: %s, %i -> %i", tlvname.toStdString().c_str(), orig_row, endRow);
			insertRow(endRow, addRow(tlvname, check_state == Qt::Checked));

			for (size_t i = 0, ie = m_observers.size(); i < ie; ++i)
			{
				QString txt = qVariantValue<QString>(m_observers.at(i)->data(m_observers.at(i)->index(orig_row, 0, QModelIndex())));
				m_observers.at(i)->removeRows(orig_row, 1);
				int const target_row = endRow > orig_row ? endRow - 1 : endRow;
				static_cast<QStandardItemModel *>(m_observers.at(i))->insertRow(target_row, addUncheckableRow(txt));
			}
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
				QStandardItem const * const item = itemFromIndex(index);
				stream << tlvname << index.row() << static_cast<int>(item->checkState());
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
	qDebug("settings, clicked idx=%i", idx);
	clearListView(ui_settings->listViewColumnSetup);
	clearListView(ui_settings->listViewColumnSizes);
	clearListView(ui_settings->listViewColumnAlign);
	clearListView(ui_settings->listViewColumnElide);

	ui_settings->listViewColumnSetup->reset();
	ui_settings->listViewColumnSizes->reset();
	ui_settings->listViewColumnAlign->reset();
	ui_settings->listViewColumnElide->reset();

	QStandardItem * cs_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnSetup->model())->invisibleRootItem();
	QStandardItem * csz_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnSizes->model())->invisibleRootItem();
	QStandardItem * cal_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnAlign->model())->invisibleRootItem();
	QStandardItem * cel_root = static_cast<QStandardItemModel *>(ui_settings->listViewColumnElide->model())->invisibleRootItem();
	if (idx < m_columns_setup.size())
		for (int i = 0, ie = m_columns_setup[idx].size(); i < ie; ++i)
		{
			cs_root->appendRow(addRow(m_columns_setup.at(idx).at(i), true));
			csz_root->appendRow(addUncheckableRow(tr("%1").arg(m_columns_sizes.at(idx).at(i))));
			cal_root->appendRow(addUncheckableRow(tr("%1").arg(m_columns_align.at(idx).at(i))));
			cel_root->appendRow(addUncheckableRow(tr("%1").arg(m_columns_elide.at(idx).at(i))));
		}

	//size_t const n = tlv::get_tag_count() - 1; // -1 is for the tag Bool
	size_t const n = tlv::tag_bool;

	size_t add_tag_count = 0;
	size_t * const add_tag_indices = static_cast<size_t * const>(alloca(sizeof(size_t) * n));
	for (size_t i = tlv::tag_time; i < n; ++i)
	{
		char const * name = tlv::get_tag_name(i);
		if (name)
		{
			if (findChildByText(cs_root, QString::fromAscii(name)))
				continue;

			QList<QStandardItem *> row_items = addRow(QString::fromAscii(name), false);
			cs_root->appendRow(row_items);
			add_tag_indices[add_tag_count++] = i;

			csz_root->appendRow(addUncheckableRow(QString("0")));
			cal_root->appendRow(addUncheckableRow(QString(aligns[0])));
			cel_root->appendRow(addUncheckableRow(QString(elides[0])));
		}
	}

	disconnect(ui_settings->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	disconnect(ui_settings->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	disconnect(ui_settings->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	disconnect(ui_settings->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));
	connect(ui_settings->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	connect(ui_settings->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	connect(ui_settings->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	connect(ui_settings->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));

	connect(ui_settings->macUserButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingPooftahButton()));
}

void MainWindow::onSetup ()
{
	ui_settings->comboBoxApp->clear();
	for (int a = 0, ae = m_app_names.size(); a < ae; ++a)
		ui_settings->comboBoxApp->addItem(m_app_names.at(a));

	int curr_app_idx = 0;
	Connection * conn = m_server->findCurrentConnection();
	if (conn)
	{
		curr_app_idx = conn->sessionState().m_app_idx;
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

	onSettingsAppSelected(curr_app_idx);

	if (m_settings_dialog->exec() == QDialog::Accepted)
	{
		for (int app_idx = 0, app_idxe = m_app_names.size(); app_idx < app_idxe; ++app_idx)
		{
			qDebug("app=%s", m_app_names.at(app_idx).toStdString().c_str());
			m_columns_setup[app_idx].clear();
			m_columns_sizes[app_idx].clear();
			m_columns_align[app_idx].clear();
			m_columns_elide[app_idx].clear();

			for (size_t j = 0, je = ui_settings->listViewColumnSetup->model()->rowCount(); j < je; ++j)
			{
				QModelIndex const row_idx = ui_settings->listViewColumnSetup->model()->index(j, 0, QModelIndex());
				QStandardItem * const item = static_cast<QStandardItemModel *>(ui_settings->listViewColumnSetup->model())->itemFromIndex(row_idx);
				if (item->checkState() == Qt::Checked)
					m_columns_setup[app_idx].append(qVariantValue<QString>(ui_settings->listViewColumnSetup->model()->data(row_idx)));
			}
			for (size_t j = 0, je = ui_settings->listViewColumnSizes->model()->rowCount(); j < je; ++j)
			{
				QModelIndex const row_idx = ui_settings->listViewColumnSizes->model()->index(j, 0, QModelIndex());
				m_columns_sizes[app_idx].append(qVariantValue<QString>(ui_settings->listViewColumnSizes->model()->data(row_idx)).toInt());
			}
			for (size_t j = 0, je = ui_settings->listViewColumnAlign->model()->rowCount(); j < je; ++j)
			{
				QModelIndex const row_idx = ui_settings->listViewColumnAlign->model()->index(j, 0, QModelIndex());
				m_columns_align[app_idx].append(qVariantValue<QString>(ui_settings->listViewColumnAlign->model()->data(row_idx)));
			}
			for (size_t j = 0, je = ui_settings->listViewColumnElide->model()->rowCount(); j < je; ++j)
			{
				QModelIndex const row_idx = ui_settings->listViewColumnElide->model()->index(j, 0, QModelIndex());
				m_columns_elide[app_idx].append(qVariantValue<QString>(ui_settings->listViewColumnElide->model()->data(row_idx)));
			}

			if (curr_app_idx == app_idx)
				if (Connection * conn = m_server->findCurrentConnection())
					conn->onApplyColumnSetup();
		}
	}

	disconnect(ui_settings->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	disconnect(ui_settings->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	disconnect(ui_settings->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	disconnect(ui_settings->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));
	clearListView(ui_settings->listViewColumnSetup);
	clearListView(ui_settings->listViewColumnSizes);
	clearListView(ui_settings->listViewColumnAlign);
	clearListView(ui_settings->listViewColumnElide);
	ui_settings->listViewColumnSetup->reset();
	ui_settings->listViewColumnSizes->reset();
	ui_settings->listViewColumnAlign->reset();
	ui_settings->listViewColumnElide->reset();
}

void MainWindow::onClickedAtSettingPooftahButton ()
{
	for (size_t j = 0, je = ui_settings->listViewColumnAlign->model()->rowCount(); j < je; ++j)
	{
		QModelIndex const tag_idx = ui_settings->listViewColumnSetup->model()->index(j, 0, QModelIndex());
		QString const tag = qVariantValue<QString>(ui_settings->listViewColumnSetup->model()->data(tag_idx));

		E_Align const default_aligns[tlv::tag_max_value] = {
			  e_AlignLeft		// invalid
			, e_AlignLeft		// app
			, e_AlignLeft		// pid
			, e_AlignRight		// time
			, e_AlignRight		// tid
			, e_AlignRight		// file
			, e_AlignRight		// line
			, e_AlignRight		// func
			, e_AlignLeft		// msg
			, e_AlignRight		// lvl
			, e_AlignRight		// ctx
			, e_AlignLeft		// bool
			, e_AlignLeft		// int
			, e_AlignLeft		// str
		};
		E_Elide const default_elides[tlv::tag_max_value] = {
			  e_ElideNone		// invalid
			, e_ElideNone		// app
			, e_ElideNone		// pid
			, e_ElideLeft		// time
			, e_ElideNone		// tid
			, e_ElideLeft		// file
			, e_ElideNone		// line
			, e_ElideLeft		// func
			, e_ElideRight		// msg
			, e_ElideLeft		// lvl
			, e_ElideLeft		// ctx
			, e_ElideNone		// bool
			, e_ElideNone		// int
			, e_ElideNone		// str
		};

		int const default_sizes[tlv::tag_max_value] = {
			  0		// invalid
			, 0		// app
			, 0		// pid
			, 64	// time
			, 16	// tid
			, 192	// file
			, 32	// line
			, 128	// func
			, 512	// msg
			, 16	// lvl
			, 16	// ctx
			, 0		// bool
			, 0		// int
			, 0		// str
		};


		QModelIndex const row_idx = ui_settings->listViewColumnAlign->model()->index(j, 0, QModelIndex());
		size_t const tag_val = tlv::tag_for_name(tag.toAscii());
		ui_settings->listViewColumnAlign->model()->setData(row_idx, QString(alignToString(default_aligns[tag_val])));

		QModelIndex const erow_idx = ui_settings->listViewColumnElide->model()->index(j, 0, QModelIndex());
		ui_settings->listViewColumnElide->model()->setData(erow_idx, QString(elideToString(default_elides[tag_val])));

		QModelIndex const srow_idx = ui_settings->listViewColumnSizes->model()->index(j, 0, QModelIndex());
		ui_settings->listViewColumnSizes->model()->setData(srow_idx, tr("%1").arg(default_sizes[tag_val]));
	}
}

#if 0
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

	// @NOTE: hmm, that not work. me not know why. this no meat bone crush it!
	/*QVariant data (QModelIndex const & index, int role = Qt::DisplayRole) const {
		if (!index.isValid()) return QVariant();
		return QVariant(m_data[index.row()]);
	}
	int rowCount (QModelIndex const & parent) const {
		if (parent.isValid()) return 0;
		else return m_data.size();
	}
	int columnCount (QModelIndex const & parent) const { return 1; }*/

			//beginInsertRows(QModelIndex(), endRow, endRow);
			//m_data.insert(endRow, tlvname);
			//endInsertRows();

#endif

