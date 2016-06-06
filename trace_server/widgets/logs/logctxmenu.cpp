#include "logctxmenu.h"
#include "logconfig.h"
#include "logwidget.h"
#include <utils/utils.h>
#include "connection.h"
#include "mainwindow.h"

namespace logs {

LogCtxMenu::LogCtxMenu (LogWidget & lw, QWidget * parent)
	: m_log_widget(lw)
	, m_ui(new ::Ui::SettingsLog)
	, m_widget(new QDockWidget(parent))
{
	m_ui->setupUi(m_widget);

	// EXPERIMENTAL
	QString const name = lw.path().join("/") + "/utils"; // @NOTE: not nice, dup is in ui_settingslog.h"
	m_widget->setObjectName(name);
	m_widget->setWindowTitle(name);
	m_widget->setAllowedAreas(Qt::NoDockWidgetArea);
	m_log_widget.m_connection->getMainWindow()->addDockWidget(Qt::RightDockWidgetArea, m_widget);
	m_widget->setFloating(true);

	//m_actionables.insert(name, this);
	m_widget->setAttribute(Qt::WA_DeleteOnClose, false);
	m_widget->setVisible(false);

	prepareSettingsWidgets();
}

void LogCtxMenu::refreshUI ()
{
	if (m_widget->isVisible())
	{
		setConfigValuesToUI(m_log_widget.m_config);
	}
}

void LogCtxMenu::onShowContextMenu (QPoint const & pos)
{
	bool const visible = m_widget->isVisible();
	m_widget->setVisible(!visible);

	if (m_widget->isVisible())
	{
		//m_log_widget.m_connection->getMainWindow()->restoreDockedWidgetGeometry();
		setConfigValuesToUI(m_log_widget.m_config);
		if (m_widget->isFloating())
			m_widget->move(pos);
	}
}


/*
void CtxLogConfig::setupSeparatorChar (QString const & c)
{
	m_ui->separatorComboBox->addItem(c);
	m_ui->separatorComboBox->setCurrentIndex(m_ui->separatorComboBox->findText(c));
}

QString CtxLogConfig::separatorChar () const
{
	return m_ui->protocolComboBox->currentText();
}
*/


void LogCtxMenu::syncSettingsViews (QListView const * const invoker, QModelIndex const idx)
{
	QListView * const views[] = { m_ui->listViewColumnSetup, m_ui->listViewColumnSizes, m_ui->listViewColumnAlign, m_ui->listViewColumnElide };
	for (size_t i = 0; i < sizeof(views) / sizeof(*views); ++i)
		if (views[i] != invoker)
		{
			views[i]->selectionModel()->clearSelection();
			QModelIndex const other_idx = views[i]->model()->index(idx.row(), idx.column(), QModelIndex());
			views[i]->selectionModel()->select(other_idx, QItemSelectionModel::Select);
		}
}

void LogCtxMenu::onClickedAtSettingColumnSetup (QModelIndex const idx)
{
	syncSettingsViews(m_ui->listViewColumnSetup, idx);
}
void LogCtxMenu::onClickedAtSettingColumnSizes (QModelIndex const idx)
{
	syncSettingsViews(m_ui->listViewColumnSizes, idx);
}
void LogCtxMenu::onClickedAtSettingColumnAlign (QModelIndex const idx)
{
	syncSettingsViews(m_ui->listViewColumnAlign, idx);
	QString const txt = m_ui->listViewColumnAlign->model()->data(idx).toString();
	E_Align act = e_AlignLeft;
	if (txt.isEmpty())
	{ }
	else
	{
		E_Align const curr = stringToAlign(txt.toStdString().c_str()[0]);
		size_t i = (curr + 1) % e_max_align_enum_value;
		act = static_cast<E_Align>(i);
	}
	m_ui->listViewColumnAlign->model()->setData(idx, QString(alignToString(act)));
}
void LogCtxMenu::onClickedAtSettingColumnElide (QModelIndex const idx)
{
	syncSettingsViews(m_ui->listViewColumnElide, idx);
	QString const txt = m_ui->listViewColumnElide->model()->data(idx).toString();
	E_Elide act = static_cast<E_Elide>(0);
	if (txt.isEmpty())
	{ }
	else
	{
		E_Elide const curr = stringToElide(txt.toStdString().c_str()[0]);
		size_t i = (curr + 1) % e_max_elide_enum_value;
		act = static_cast<E_Elide>(i);
	}
	m_ui->listViewColumnElide->model()->setData(idx, QString(elideToString(act)));
}

void LogCtxMenu::prepareSettingsWidgets ()
{
	/*MyListModel * model = new MyListModel(this);*/
	m_ui->listViewColumnSetup->setModel(new QStandardItemModel(this));
	m_ui->listViewColumnSizes->setModel(new QStandardItemModel(this));
	m_ui->listViewColumnAlign->setModel(new QStandardItemModel(this));
	m_ui->listViewColumnElide->setModel(new QStandardItemModel(this));
	m_ui->listViewColumnAlign->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_ui->listViewColumnElide->setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(m_ui->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	connect(m_ui->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	connect(m_ui->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	connect(m_ui->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));

	connect(m_ui->autoSetupButton, SIGNAL(clicked()), this, SLOT(onClickedAtAutoSetupButton()));
	connect(m_ui->applyButton, SIGNAL(clicked()), this, SLOT(onClickedAtApplyButton()));
	//connect(m_ui->defaultsButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingDefaultsButton()));
}


void LogCtxMenu::setConfigValuesToUI (LogConfig const & cfg)
{
	clearUI();

	QStandardItem * cs_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnSetup->model())->invisibleRootItem();
	QStandardItem * csz_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnSizes->model())->invisibleRootItem();
	QStandardItem * cal_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnAlign->model())->invisibleRootItem();
	QStandardItem * cel_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnElide->model())->invisibleRootItem();
	for (int i = 0, ie = cfg.m_columns_setup.size(); i < ie; ++i)
	{
		int const li = m_log_widget.m_tableview->horizontalHeader()->logicalIndex(i);
		Q_ASSERT(li > -1);
    if (li == -1)
    {
      qWarning("li == -1 for i=%i", i);
      continue;
    }

		//bool const hidden = m_log_widget.horizontalHeader()->isSectionHidden(li);
		cs_root->appendRow(addUncheckableRow(tr("%1").arg(cfg.m_columns_setup.at(li))));
		csz_root->appendRow(addUncheckableRow(tr("%1").arg(cfg.m_columns_sizes.at(li))));
		cal_root->appendRow(addUncheckableRow(tr("%1").arg(cfg.m_columns_align.at(li))));
		cel_root->appendRow(addUncheckableRow(tr("%1").arg(cfg.m_columns_elide.at(li))));
	}

	size_t const n = proto::get_tag_count();
	for (size_t i = 0; i < n; ++i)
	{
		char const * name = proto::get_tag_name(i);
		if (name)
		{
			if (findChildByText(cs_root, QString::fromLatin1(name)))
				continue;

			QList<QStandardItem *> row_items = addUncheckableRow(QString::fromLatin1(name));
			cs_root->appendRow(row_items);

			TagDesc const & td = cfg.m_tag_config.findTag(i);
			csz_root->appendRow(addUncheckableRow(tr("%1").arg(td.m_size)));
			cal_root->appendRow(addUncheckableRow(td.m_align_str));
			cel_root->appendRow(addUncheckableRow(td.m_elide_str));
		}
	}
}

void LogCtxMenu::onClickedAtAutoSetupButton_noreorder ()
{
	for (int j = 0, je = m_ui->listViewColumnAlign->model()->rowCount(); j < je; ++j)
	{
		//QModelIndex const tag_idx = m_ui->listViewColumnSetup->model()->index(j, 0, QModelIndex());
		//QString const tag = m_ui->listViewColumnSetup->model()->data(tag_idx).toString();

		QModelIndex const tag_idx = m_ui->listViewColumnSetup->model()->index(j, 0, QModelIndex());
		QString const tag = m_ui->listViewColumnSetup->model()->data(tag_idx).toString();

		QModelIndex const row_idx = m_ui->listViewColumnAlign->model()->index(j, 0, QModelIndex());
		size_t const tag_val = proto::tag_for_name(tag.toLatin1());
		TagDesc const & td = m_log_widget.m_config.m_tag_config.findTag(tag_val);

		m_ui->listViewColumnAlign->model()->setData(row_idx, td.m_align_str);

		QModelIndex const erow_idx = m_ui->listViewColumnElide->model()->index(j, 0, QModelIndex());
		m_ui->listViewColumnElide->model()->setData(erow_idx, td.m_elide_str);

		QModelIndex const srow_idx = m_ui->listViewColumnSizes->model()->index(j, 0, QModelIndex());
		m_ui->listViewColumnSizes->model()->setData(srow_idx, tr("%1").arg(td.m_size));
	}
}



void LogCtxMenu::onSettingsAppSelectedCSV (int const columns, bool const first_time)
{
/*	qDebug("settings, csv");
	clearUI();

	QStandardItem * csh_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnShow->model())->invisibleRootItem();
	QStandardItem * cs_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnSetup->model())->invisibleRootItem();
	QStandardItem * csz_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnSizes->model())->invisibleRootItem();
	QStandardItem * cal_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnAlign->model())->invisibleRootItem();
	QStandardItem * cel_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnElide->model())->invisibleRootItem();
	for (int i = 0, ie = m_log_widget.m_config.m_columns_setup.size(); i < ie; ++i)
	{
		// @FIXME VIS IDX!
		cs_root->appendRow(addRow(m_log_widget.m_config.m_columns_setup.at(i), true));
		csz_root->appendRow(addUncheckableRow(tr("%1").arg(m_log_widget.m_config.m_columns_sizes.at(i))));
		cal_root->appendRow(addUncheckableRow(tr("%1").arg(m_log_widget.m_config.m_columns_align.at(i))));
		cel_root->appendRow(addUncheckableRow(tr("%1").arg(m_log_widget.m_config.m_columns_elide.at(i))));
	}
*/
	/*
	//size_t const n = proto::get_tag_count() - 1; // -1 is for the tag Bool
	size_t const n = proto::tag_bool;

	size_t add_tag_count = 0;
	size_t * const add_tag_indices = static_cast<size_t * const>(alloca(sizeof(size_t) * n));
	for (size_t i = proto::tag_time; i < n; ++i)
	{
		char const * name = proto::get_tag_name(i);
		if (name)
		{
			if (findChildByText(cs_root, QString::fromAscii(name)))
				continue;

			QList<QStandardItem *> row_items = addRow(QString::fromAscii(name), first_time);
			cs_root->appendRow(row_items);
			add_tag_indices[add_tag_count++] = i;

			csz_root->appendRow(addUncheckableRow(QString("0")));
			cal_root->appendRow(addUncheckableRow(QString(aligns[0])));
			cel_root->appendRow(addUncheckableRow(QString(elides[0])));
		}
	}*/

/*	disconnect(m_ui->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	disconnect(m_ui->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	disconnect(m_ui->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	disconnect(m_ui->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));
	connect(m_ui->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	connect(m_ui->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	connect(m_ui->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	connect(m_ui->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));
*/
	/*connect(m_ui->macUserButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingPooftahButton()));
	connect(m_ui->okButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingOkButton()));
	connect(m_ui->okSaveButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingOkSaveButton()));
	connect(m_ui->cancelButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingCancelButton()));*/

	//connect(m_ui->applyButton, SIGNAL(clicked()), this, SLOT(onClickedAtApplyButton()));
	//connect(m_ui->saveButton, SIGNAL(clicked()), this, SLOT(onClickedAtSaveButton()));
	//connect(m_ui->cancelButton, SIGNAL(clicked()), this, SLOT(onClickedAtCancelButton()));
}

void LogCtxMenu::clearUI ()
{
	clearListView(m_ui->listViewColumnSetup);
	clearListView(m_ui->listViewColumnSizes);
	clearListView(m_ui->listViewColumnAlign);
	clearListView(m_ui->listViewColumnElide);
	m_ui->listViewColumnSetup->reset();
	m_ui->listViewColumnSizes->reset();
	m_ui->listViewColumnAlign->reset();
	m_ui->listViewColumnElide->reset();
}

void LogCtxMenu::onCommitTagData (QString const & s)
{
	QStandardItem * cs_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnSetup->model())->invisibleRootItem();
	QStandardItem * csz_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnSizes->model())->invisibleRootItem();
	QStandardItem * cal_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnAlign->model())->invisibleRootItem();
	QStandardItem * cel_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnElide->model())->invisibleRootItem();

	QStandardItem * it = findChildByText(cs_root, s);
	if (!it)
		return;
	QModelIndex const idx = m_ui->listViewColumnSetup->currentIndex();

	proto::tags const t = proto::tag_for_name(s.toLatin1());
	TagDesc const & desc = m_log_widget.m_config.m_tag_config.findOrCreateTag(t);

	csz_root->child(idx.row())->setData(tr("%1").arg(desc.m_size), Qt::EditRole);
	cal_root->child(idx.row())->setData(tr("%1").arg(desc.m_align_str), Qt::EditRole);
	cel_root->child(idx.row())->setData(tr("%1").arg(desc.m_elide_str), Qt::EditRole);
}

void LogCtxMenu::onClickedAtAutoSetupButton ()
{
	LogConfig cfg;
	m_log_widget.reconfigureConfig(cfg);
	clearUI();

	QStandardItem * cs_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnSetup->model())->invisibleRootItem();
	QStandardItem * csz_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnSizes->model())->invisibleRootItem();
	QStandardItem * cal_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnAlign->model())->invisibleRootItem();
	QStandardItem * cel_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnElide->model())->invisibleRootItem();
	for (int i = 0, ie = cfg.m_columns_setup.size(); i < ie; ++i)
	{
		cs_root->appendRow(addUncheckableRow(tr("%1").arg(cfg.m_columns_setup.at(i))));
		csz_root->appendRow(addUncheckableRow(tr("%1").arg(cfg.m_columns_sizes.at(i))));
		cal_root->appendRow(addUncheckableRow(tr("%1").arg(cfg.m_columns_align.at(i))));
		cel_root->appendRow(addUncheckableRow(tr("%1").arg(cfg.m_columns_elide.at(i))));
	}
}

void LogCtxMenu::onClickedAtApplyButton ()
{
// 	LogConfig config;
// 	//qDebug("app=%s", m_config.m_app_names.at(app_idx).toStdString().c_str());
// 	config.m_columns_setup.clear();
// 	config.m_columns_sizes.clear();
// 	config.m_columns_align.clear();
// 	config.m_columns_elide.clear();
// 
// 	for (size_t j = 0, je = m_ui->listViewColumnShow->model()->rowCount(); j < je; ++j)
// 	{
// 		QModelIndex const row_idx = m_ui->listViewColumnShow->model()->index(j, 0, QModelIndex());
// 		QStandardItem * const item = static_cast<QStandardItemModel *>(m_ui->listViewColumnShow->model())->itemFromIndex(row_idx);
// 		if (item->checkState() == Qt::Checked)
// 		{
// 			{
// 				QModelIndex const row_idx = m_ui->listViewColumnSetup->model()->index(j, 0, QModelIndex());
// 				QString const & d = m_ui->listViewColumnSetup->model()->data(row_idx).toString();
// 				config.m_columns_setup.push_back(d);
// 			}
// 
// 			{
// 				QModelIndex const row_idx = m_ui->listViewColumnSizes->model()->index(j, 0, QModelIndex());
// 				config.m_columns_sizes.push_back(m_ui->listViewColumnSizes->model()->data(row_idx).toString().toInt());
// 			}
// 
// 			{
// 				QModelIndex const row_idx = m_ui->listViewColumnAlign->model()->index(j, 0, QModelIndex());
// 				config.m_columns_align.push_back(m_ui->listViewColumnAlign->model()->data(row_idx).toString());
// 			}
// 
// 			{
// 				QModelIndex const row_idx = m_ui->listViewColumnElide->model()->index(j, 0, QModelIndex());
// 				config.m_columns_elide.push_back(m_ui->listViewColumnElide->model()->data(row_idx).toString());
// 			}
// 		}
// 	}
// 
// 	/*for (size_t j = 0, je = m_ui->listViewColumnShow->model()->rowCount(); j < je; ++j)
// 	{
// 		QModelIndex const row_idx = m_ui->listViewColumnShow->model()->index(j, 0, QModelIndex());
// 		QStandardItem * const item = static_cast<QStandardItemModel *>(m_ui->listViewColumnShow->model())->itemFromIndex(row_idx);
// 		if (item->checkState() == Qt::Unchecked)
// 			config.m_columns_sizes[j] = 0;
// 	}*/
// 
// 	if (validateConfig(config))
// 	{
// 		m_log_widget.reloadModelAccordingTo(config);
// 		//m_log_widget.resizeModelToConfig(config);
// 
// 		// reorder columns and set to main config
// 		//m_log_widget.swapSectionsAccordingTo(config);
// 		/*for (int c = 0, ce = config.m_columns_sizes.size(); c < ce; ++c)
// 		{
// 			int const li = m_log_widget.horizontalHeader()->logicalIndex(c);
// 			if (li == -1)
// 			{
// 				qWarning("li == -1 for col=%i", c);
// 				continue; // @FIXME: this only hotfix
// 			}
// 			m_log_widget.m_config.m_columns_align[li] = config.m_columns_align[c];
// 			m_log_widget.m_config.m_columns_elide[li] = config.m_columns_elide[c];
// 		}*/
// 	}
//   else
//   {
// 		fillDefaultConfig(config);
// 		m_log_widget.reloadModelAccordingTo(config);
//   }
}

void LogCtxMenu::onClickedAtSaveButton ()
{
	//onClickedAtSettingOkButton();
	//storeState();
}

void LogCtxMenu::onClickedAtCancelButton ()
{
	//m_settings_dialog->close();
}

	//filterMenu->addAction(tr("Hide previous rows"), m_server, SLOT(onHidePrevFromRow()), QKeySequence(Qt::Key_Delete));
	//filterMenu->addAction(tr("Unhide previous rows"), m_server, SLOT(onUnhidePrevFromRow()), QKeySequence(Qt::ControlModifier + Qt::Key_Delete));
	//filterMenu->addAction(tr("Set time reference row"), m_server, SLOT(onTimeRefFromRow()), QKeySequence(Qt::Key_Space));
	//filterMenu->addAction(tr("Exclude file:line row"), m_server, SLOT(onExcludeFileLine()), QKeySequence(Qt::Key_X));

/*void LogTableView::onFileColOrExp (QModelIndex const & idx, bool collapsed)
{
	QStandardItemModel const * const model = static_cast<QStandardItemModel *>(filterMgr()->getFilterFileLine()->getWidgetFile()->model());
	QStandardItem * const node = model->itemFromIndex(idx);

	std::vector<QString> s;	// @TODO: hey piggy, to member variables
	s.clear();
	s.reserve(16);
	QStandardItem * parent = node;
	QModelIndex parent_idx = model->indexFromItem(parent);
	while (parent_idx.isValid())
	{
		QString const & val = model->data(parent_idx, Qt::DisplayRole).toString();
		s.push_back(val);
		parent = parent->parent();
		parent_idx = model->indexFromItem(parent);
	}

	QString file;
	for (std::vector<QString>::const_reverse_iterator it=s.rbegin(), ite=s.rend(); it != ite; ++it)
		file += QString("/") + *it;

	filterMgr()->getFilterFileLine()->m_data.set_to_state(file, TreeModelItem(static_cast<E_NodeStates>(node->checkState()), collapsed));
}*/

}

