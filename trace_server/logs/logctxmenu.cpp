#include "logctxmenu.h"
#include "logconfig.h"
#include "logwidget.h"
#include "utils.h"
#include <logs/tagconfig.h>

namespace logs {

LogCtxMenu::LogCtxMenu (LogWidget & lw, QWidget * parent)
	: m_log_widget(lw)
	, m_ui(new Ui::SettingsLog)
	, m_widget(new QDockWidget(parent))
{
	m_widget->setVisible(false);
	m_ui->setupUi(m_widget);
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
	{
		if (views[i] != invoker)
		{
			views[i]->selectionModel()->clearSelection();
			QModelIndex const other_idx = views[i]->model()->index(idx.row(), idx.column(), QModelIndex());
			views[i]->selectionModel()->select(other_idx, QItemSelectionModel::Select);
		}
	}
}

void LogCtxMenu::onClickedAtSettingColumnSetup (QModelIndex const idx)
{
	syncSettingsViews(m_ui->listViewColumnSetup, idx);

	QStandardItem * const item = static_cast<QStandardItemModel *>(m_ui->listViewColumnSetup->model())->itemFromIndex(idx);
	Qt::CheckState const curr = item->checkState();

	item->setCheckState(curr == Qt::Checked ? Qt::Unchecked : Qt::Checked);

	QModelIndex const size_idx = m_ui->listViewColumnSizes->model()->index(idx.row(), idx.column(), QModelIndex());
	if (curr == Qt::Checked)
	{
		//m_ui->listViewColumnSizes->model()->setData(size_idx, QString("0"));
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
				m_ui->listViewColumnSizes->model()->setData(size_idx, tr("%1").arg(size_val));
			}
		}*/
	}
}
void LogCtxMenu::onClickedAtSettingColumnSizes (QModelIndex const idx)
{
	syncSettingsViews(m_ui->listViewColumnSizes, idx);
}
void LogCtxMenu::onClickedAtSettingColumnAlign (QModelIndex const idx)
{
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
	syncSettingsViews(m_ui->listViewColumnAlign, idx);
}
void LogCtxMenu::onClickedAtSettingColumnElide (QModelIndex const idx)
{
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

	syncSettingsViews(m_ui->listViewColumnElide, idx);
}

void LogCtxMenu::onSettingsAppSelectedTLV (int const idx, bool const first_time)
{
	qDebug("settings, clicked idx=%i", idx);
	clearListView(m_ui->listViewColumnSetup);
	clearListView(m_ui->listViewColumnSizes);
	clearListView(m_ui->listViewColumnAlign);
	clearListView(m_ui->listViewColumnElide);

	m_ui->listViewColumnSetup->reset();
	m_ui->listViewColumnSizes->reset();
	m_ui->listViewColumnAlign->reset();
	m_ui->listViewColumnElide->reset();

	QStandardItem * cs_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnSetup->model())->invisibleRootItem();
	QStandardItem * csz_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnSizes->model())->invisibleRootItem();
	QStandardItem * cal_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnAlign->model())->invisibleRootItem();
	QStandardItem * cel_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnElide->model())->invisibleRootItem();
	for (int i = 0, ie = m_log_widget.m_config.m_columns_setup.size(); i < ie; ++i)
	{
		cs_root->appendRow(addRow(m_log_widget.m_config.m_columns_setup.at(i), true));
		csz_root->appendRow(addUncheckableRow(tr("%1").arg(m_log_widget.m_config.m_columns_sizes.at(i))));
		cal_root->appendRow(addUncheckableRow(tr("%1").arg(m_log_widget.m_config.m_columns_align.at(i))));
		cel_root->appendRow(addUncheckableRow(tr("%1").arg(m_log_widget.m_config.m_columns_elide.at(i))));
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
			if (findChildByText(cs_root, QString::fromLatin1(name)))
				continue;

			QList<QStandardItem *> row_items = addRow(QString::fromLatin1(name), first_time);
			cs_root->appendRow(row_items);
			add_tag_indices[add_tag_count++] = i;

			csz_root->appendRow(addUncheckableRow(QString("0")));
			cal_root->appendRow(addUncheckableRow(QString(aligns[0])));
			cel_root->appendRow(addUncheckableRow(QString(elides[0])));
		}
	}

	disconnect(m_ui->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	disconnect(m_ui->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	disconnect(m_ui->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	disconnect(m_ui->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));
	connect(m_ui->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	connect(m_ui->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	connect(m_ui->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	connect(m_ui->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));

	connect(m_ui->autoSetupButton, SIGNAL(clicked()), this, SLOT(onClickedAtAutoSetupButton()));
	connect(m_ui->applyButton, SIGNAL(clicked()), this, SLOT(onClickedAtApplyButton()));
	connect(m_ui->saveButton, SIGNAL(clicked()), this, SLOT(onClickedAtSaveButton()));
	//connect(m_ui->cancelButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingCancelButton()));
}


void LogCtxMenu::onSettingsAppSelectedCSV (int const idx, int const columns, bool const first_time)
{
	qDebug("settings, clicked idx=%i", idx);
	clearListView(m_ui->listViewColumnSetup);
	clearListView(m_ui->listViewColumnSizes);
	clearListView(m_ui->listViewColumnAlign);
	clearListView(m_ui->listViewColumnElide);

	m_ui->listViewColumnSetup->reset();
	m_ui->listViewColumnSizes->reset();
	m_ui->listViewColumnAlign->reset();
	m_ui->listViewColumnElide->reset();

	QStandardItem * cs_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnSetup->model())->invisibleRootItem();
	QStandardItem * csz_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnSizes->model())->invisibleRootItem();
	QStandardItem * cal_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnAlign->model())->invisibleRootItem();
	QStandardItem * cel_root = static_cast<QStandardItemModel *>(m_ui->listViewColumnElide->model())->invisibleRootItem();
	for (int i = 0, ie = m_log_widget.m_config.m_columns_setup.size(); i < ie; ++i)
	{
		cs_root->appendRow(addRow(m_log_widget.m_config.m_columns_setup.at(i), true));
		csz_root->appendRow(addUncheckableRow(tr("%1").arg(m_log_widget.m_config.m_columns_sizes.at(i))));
		cal_root->appendRow(addUncheckableRow(tr("%1").arg(m_log_widget.m_config.m_columns_align.at(i))));
		cel_root->appendRow(addUncheckableRow(tr("%1").arg(m_log_widget.m_config.m_columns_elide.at(i))));
	}

	/*
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

			QList<QStandardItem *> row_items = addRow(QString::fromAscii(name), first_time);
			cs_root->appendRow(row_items);
			add_tag_indices[add_tag_count++] = i;

			csz_root->appendRow(addUncheckableRow(QString("0")));
			cal_root->appendRow(addUncheckableRow(QString(aligns[0])));
			cel_root->appendRow(addUncheckableRow(QString(elides[0])));
		}
	}*/

	disconnect(m_ui->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	disconnect(m_ui->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	disconnect(m_ui->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	disconnect(m_ui->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));
	connect(m_ui->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	connect(m_ui->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	connect(m_ui->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	connect(m_ui->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));

	/*connect(m_ui->macUserButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingPooftahButton()));
	connect(m_ui->okButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingOkButton()));
	connect(m_ui->okSaveButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingOkSaveButton()));
	connect(m_ui->cancelButton, SIGNAL(clicked()), this, SLOT(onClickedAtSettingCancelButton()));*/

	connect(m_ui->applyButton, SIGNAL(clicked()), this, SLOT(onClickedAtApplyButton()));
	connect(m_ui->saveButton, SIGNAL(clicked()), this, SLOT(onClickedAtSaveButton()));
	//connect(m_ui->cancelButton, SIGNAL(clicked()), this, SLOT(onClickedAtCancelButton()));
}

void LogCtxMenu::clearSettingWidgets ()
{
	disconnect(m_ui->listViewColumnSetup, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSetup(QModelIndex)));
	disconnect(m_ui->listViewColumnSizes, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnSizes(QModelIndex)));
	disconnect(m_ui->listViewColumnAlign, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnAlign(QModelIndex)));
	disconnect(m_ui->listViewColumnElide, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedAtSettingColumnElide(QModelIndex)));
	clearListView(m_ui->listViewColumnSetup);
	clearListView(m_ui->listViewColumnSizes);
	clearListView(m_ui->listViewColumnAlign);
	clearListView(m_ui->listViewColumnElide);
	m_ui->listViewColumnSetup->reset();
	m_ui->listViewColumnSizes->reset();
	m_ui->listViewColumnAlign->reset();
	m_ui->listViewColumnElide->reset();
}



void LogCtxMenu::onClickedAtAutoSetupButton ()
{
	for (size_t j = 0, je = m_ui->listViewColumnAlign->model()->rowCount(); j < je; ++j)
	{
		QModelIndex const tag_idx = m_ui->listViewColumnSetup->model()->index(j, 0, QModelIndex());
		QString const tag = m_ui->listViewColumnSetup->model()->data(tag_idx).toString();

		QModelIndex const row_idx = m_ui->listViewColumnAlign->model()->index(j, 0, QModelIndex());
		size_t const tag_val = tlv::tag_for_name(tag.toLatin1());
		m_ui->listViewColumnAlign->model()->setData(row_idx, QString(alignToString(default_aligns[tag_val])));

		QModelIndex const erow_idx = m_ui->listViewColumnElide->model()->index(j, 0, QModelIndex());
		m_ui->listViewColumnElide->model()->setData(erow_idx, QString(elideToString(default_elides[tag_val])));

		QModelIndex const srow_idx = m_ui->listViewColumnSizes->model()->index(j, 0, QModelIndex());
		m_ui->listViewColumnSizes->model()->setData(srow_idx, tr("%1").arg(default_sizes[tag_val]));
	}
}

void LogCtxMenu::onClickedAtApplyButton ()
{
	for (int app_idx = 0, app_idxe = m_config.m_app_names.size(); app_idx < app_idxe; ++app_idx)
	{
		qDebug("app=%s", m_config.m_app_names.at(app_idx).toStdString().c_str());
		m_config.m_columns_setup[app_idx].clear();
		m_config.m_columns_sizes[app_idx].clear();
		m_config.m_columns_align[app_idx].clear();
		m_config.m_columns_elide[app_idx].clear();

		for (size_t j = 0, je = m_ui->listViewColumnSetup->model()->rowCount(); j < je; ++j)
		{
			QModelIndex const row_idx = m_ui->listViewColumnSetup->model()->index(j, 0, QModelIndex());
			QStandardItem * const item = static_cast<QStandardItemModel *>(m_ui->listViewColumnSetup->model())->itemFromIndex(row_idx);
			if (item->checkState() == Qt::Checked)
			{
				QString const & d = m_ui->listViewColumnSetup->model()->data(row_idx).toString();
				m_config.m_columns_setup[app_idx].append(d);
			}
		}
		for (size_t j = 0, je = m_ui->listViewColumnSizes->model()->rowCount(); j < je; ++j)
		{
			QModelIndex const row_idx = m_ui->listViewColumnSizes->model()->index(j, 0, QModelIndex());
			m_config.m_columns_sizes[app_idx].append(m_ui->listViewColumnSizes->model()->data(row_idx).toString().toInt());
		}
		for (size_t j = 0, je = m_ui->listViewColumnAlign->model()->rowCount(); j < je; ++j)
		{
			QModelIndex const row_idx = m_ui->listViewColumnAlign->model()->index(j, 0, QModelIndex());
			m_config.m_columns_align[app_idx].append(m_ui->listViewColumnAlign->model()->data(row_idx).toString());
		}
		for (size_t j = 0, je = m_ui->listViewColumnElide->model()->rowCount(); j < je; ++j)
		{
			QModelIndex const row_idx = m_ui->listViewColumnElide->model()->index(j, 0, QModelIndex());
			m_config.m_columns_elide[app_idx].append(m_ui->listViewColumnElide->model()->data(row_idx).toString());
		}
	}

	//m_settings_dialog->close();
	m_server->onApplyColumnSetup();
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

}

