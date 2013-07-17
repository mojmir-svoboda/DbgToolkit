#include "connection.h"
#include <QStandardItemModel>
#include <QListView>
#include <QHeaderView>
#include <tlv_parser/tlv_encoder.h>
#include "logs/logtablemodel.h"
#include "utils.h"
#include "utils_qstandarditem.h"
#include "constants.h"
//#include "statswindow.h"
#include "delegates.h"

void Connection::handleCSVSetup (QString const & fname)
{
/*	qDebug("Connection::handleCSVSetup() this=0x%08x", this);

	this->setupModelColorRegex();
	this->setupModelRegex();
	this->setupModelString();

	QString app_name = fname;
	if (m_main_window->reuseTabEnabled())
	{
		Connection * conn = m_main_window->findConnectionByName(app_name);
		if (conn)
		{
			if (!m_main_window->clrFltEnabled())
			{
				loadSessionState(conn->sessionState(), m_session_state);
			}

			QWidget * w = conn->m_tab_widget;
			m_main_window->onCloseTab(w);	// close old one
			// @TODO: delete persistent storage for the tab
		}
		else
		{
			QString const pname = m_main_window->matchClosestPresetName(app_name);
			m_main_window->onPresetActivate(this, pname);
		}
	}

	m_app_name = app_name;
	//sessionState().m_pid = pid;

	m_table_view_widget->setVisible(false);
	int const tab_idx = m_main_window->getTabTrace()->indexOf(m_tab_widget);
	m_main_window->getTabTrace()->setTabText(tab_idx, app_name);

	m_app_idx = m_main_window->findAppName(app_name);
	if (m_app_idx == e_InvalidItem)
	{
		qDebug("Unknown application: requesting user input");
		m_app_idx = m_main_window->createAppName(app_name, e_Proto_CSV);
	}

	columns_setup_t & cs_setup = m_main_window->getColumnSetup(sessionState().m_app_idx);
	columns_sizes_t & cs_sizes = m_main_window->getColumnSizes(sessionState().m_app_idx);
	columns_align_t & cs_align = m_main_window->getColumnAlign(sessionState().m_app_idx);
	columns_elide_t & cs_elide = m_main_window->getColumnElide(sessionState().m_app_idx);

	if (cs_setup.empty() || cs_sizes.empty() || cs_align.empty() || cs_elide.empty())
	{
		//m_main_window->onSetup(sessionState().m_app_idx, true, true);
	}

	sessionState().setupColumnsCSV(&cs_setup, &cs_sizes, &cs_align, &cs_elide); 

	m_current_cmd.tvs.reserve(sessionState().getColumnsSetupCurrent()->size());
	for (size_t i = 0, ie = sessionState().getColumnsSetupCurrent()->size(); i < ie; ++i)
	{
		m_table_view_widget->model()->insertColumn(i);
	}

	m_table_view_widget->horizontalHeader()->resizeSections(QHeaderView::Fixed);

	columns_sizes_t const & sizes = *sessionState().m_columns_sizes;
	for (int c = 0, ce = sizes.size(); c < ce; ++c)
	{
		m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
		//qDebug("sizes: %u %u %u", sizes.at(0), sizes.at(1), sizes.at(2));
	}

	m_table_view_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_table_view_widget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	//connect(m_table_view_widget, SIGNAL(clicked(QModelIndex const &)), this, SLOT(onTableClicked(QModelIndex const &)));
	//connect(m_table_view_widget, SIGNAL(doubleClicked(QModelIndex const &)), this, SLOT(onTableDoubleClicked(QModelIndex const &)));
	//m_table_view_widget->setContextMenuPolicy(Qt::CustomContextMenu);
	//connect(m_table_view_widget, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

	m_table_view_widget->setVisible(true);
	//m_table_view_widget->setItemDelegate(new TableItemDelegate(sessionState(), this));

	m_main_window->getTabTrace()->setCurrentIndex(tab_idx);
	static_cast<LogTableModel *>(m_table_view_widget->model())->emitLayoutChanged();

	qDebug("Server::incomingConnection buffering not enabled, notifying client");
	onBufferingStateChanged(m_main_window->buffState());*/
}

void Connection::tryLoadMatchingPreset (QString const & app_name)
{
	//m_file_model->beforeLoad();
	QString const preset_name = m_main_window->matchClosestPresetName(app_name);
	m_main_window->onPresetActivate(this, preset_name);
	//m_file_model->afterLoad();
}

bool Connection::handleSetupCommand (DecodedCommand const & cmd)
{
	qDebug("Connection::handleSetupCommand() this=0x%08x", this);

	QString pid;
	if (m_main_window->dumpModeEnabled())
	{
		for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
			if (cmd.tvs[i].m_tag == tlv::tag_pid)
				pid = cmd.tvs[i].m_val;
	}

	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_lvl)
		{
			int const client_level = cmd.tvs[i].m_val.toInt();
			int const server_level = m_main_window->getLevel();
			if (client_level != server_level)
			{
				qDebug("notifying client about new level");
				onLevelValueChanged(server_level);
			}
		}

		if (cmd.tvs[i].m_tag == tlv::tag_app)
		{
			/*this->setupModelFile();
			this->setupModelLvl();
			this->setupModelCtx();
			this->setupModelTID();
			this->setupModelColorRegex();
			this->setupModelRegex();
			this->setupModelString();*/

			QString app_name = cmd.tvs[i].m_val;
			if (m_main_window->reuseTabEnabled())
			{
				Connection * conn = m_main_window->findConnectionByName(app_name);
				if (conn)
				{
					qDebug("cmd setup: looking for app=%s: not found", app_name.toStdString().c_str());
					/*if (!m_main_window->clrFltEnabled())
					{
						m_file_model->beforeLoad();
						loadSessionState(conn->sessionState(), m_session_state);
					}*/

					QWidget * w = conn->m_tab_widget;
					m_main_window->onCloseTab(w);	// close old one
					// @TODO: delete persistent storage for the tab

					//m_file_model->afterLoad();
				}
				else
				{
					qDebug("cmd setup: looking for app=%s: found", app_name.toStdString().c_str());
					//m_file_model->beforeLoad();
					QString const pname = m_main_window->matchClosestPresetName(app_name);
					m_main_window->onPresetActivate(this, pname);
					//m_file_model->afterLoad();
				}

			}
			else
			{
				tryLoadMatchingPreset(app_name);
			}


			/*{
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetColorRegex()->model());
				if (model)
				{
					if (QStandardItem * root = model->invisibleRootItem())
					{
						for (int i = 0; i < sessionState().m_colorized_texts.size(); ++i)
						{
							ColorizedText & ct = sessionState().m_colorized_texts[i];
							ct.m_regex = QRegExp(ct.m_regex_str);

							QStandardItem * child = findChildByText(root, ct.m_regex_str);
							if (child == 0)
							{
								QList<QStandardItem *> row_items = addRow(ct.m_regex_str, ct.m_is_enabled);
								root->appendRow(row_items);
							}
						}
						recompileColorRegexps();
					}
					else
						qWarning("cregexp - nonexistent root");
				}
				else
					qWarning("cregexp - nonexistent model");

			}

			{
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetRegex()->model());
				if (model)
				{
					if (QStandardItem * root = model->invisibleRootItem())
					{
						for (int i = 0; i < sessionState().m_filtered_regexps.size(); ++i)
						{
							FilteredRegex & flt = sessionState().m_filtered_regexps[i];
							flt.m_regex = QRegExp(flt.m_regex_str);

							QStandardItem * child = findChildByText(root, flt.m_regex_str);
							if (child == 0)
							{
								Qt::CheckState const state = flt.m_is_enabled ? Qt::Checked : Qt::Unchecked;
								QList<QStandardItem *> row_items = addTriRow(flt.m_regex_str, state, static_cast<bool>(flt.m_state));
								root->appendRow(row_items);
								child = findChildByText(root, flt.m_regex_str);
								child->setCheckState(state);
							}
						}
						recompileRegexps();
					}
					else
						qWarning("regexp - nonexistent root");
				}
				else
					qWarning("regexp - nonexistent model");
			}

			{
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetLvl()->model());
				if (model)
				{
					if (QStandardItem * root = model->invisibleRootItem())
					{
						std::sort(sessionState().m_lvl_filters.begin(), sessionState().m_lvl_filters.end());
						for (int i = 0; i < sessionState().m_lvl_filters.size(); ++i)
						{
							FilteredLevel & flt = sessionState().m_lvl_filters[i];
							appendToLvlWidgets(flt);
						}
					}
					else
						qWarning("lvl - nonexistent root");
				}
				else
					qWarning("lvl - nonexistent model");

			}
			{
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetCtx()->model());
				if (model)
				{
					if (QStandardItem * root = model->invisibleRootItem())
					{
						for (int i = 0; i < sessionState().m_ctx_filters.size(); ++i)
						{
							FilteredContext & flt = sessionState().m_ctx_filters[i];
							appendToCtxWidgets(flt);
						}
					}
					else
						qWarning("ctx - nonexistent root");
				}
				else
					qWarning("ctx - nonexistent model");
			}
			{
				QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetString()->model());
				if (model)
				{
					if (QStandardItem * root = model->invisibleRootItem())
						for (int i = 0; i < sessionState().m_filtered_strings.size(); ++i)
						{
							FilteredString & flt = sessionState().m_filtered_strings[i];
							appendToStringWidgets(flt);
						}
					else
						qWarning("str - nonexistent root");
				}
				else
					qWarning("str - nonexistent model");
			}*/

			//this->setupModelFile();
			//this->setupModelLvl();

			m_app_name = app_name;
			//sessionState().m_pid = pid;

			int const tab_idx = m_main_window->getTabTrace()->indexOf(m_tab_widget);
			m_main_window->getTabTrace()->setTabText(tab_idx, app_name);
			QString storage_name = createStorageName();
			setupStorage(storage_name);

			m_app_idx = m_main_window->findAppName(app_name);
			if (m_app_idx == e_InvalidItem)
			{
				m_app_idx = m_main_window->createAppName(app_name, e_Proto_TLV);
			}

			/*columns_setup_t & cs_setup = m_main_window->getColumnSetup(sessionState().m_app_idx);
			columns_sizes_t & cs_sizes = m_main_window->getColumnSizes(sessionState().m_app_idx);
			columns_align_t & cs_align = m_main_window->getColumnAlign(sessionState().m_app_idx);
			columns_elide_t & cs_elide = m_main_window->getColumnElide(sessionState().m_app_idx);

			if (cs_setup.empty() || cs_sizes.empty() || cs_align.empty() || cs_elide.empty())
			{
				m_main_window->onSetup(e_Proto_TLV, sessionState().m_app_idx, true, true);
			}

			sessionState().setupColumns(&cs_setup, &cs_sizes, &cs_align, &cs_elide); */

			/*
			m_current_cmd.tvs.reserve(sessionState().getColumnsSetupCurrent()->size());
			for (size_t i = 0, ie = sessionState().getColumnsSetupCurrent()->size(); i < ie; ++i)
			{
				m_table_view_widget->model()->insertColumn(i);
			}

			m_table_view_widget->horizontalHeader()->resizeSections(QHeaderView::Fixed);

			columns_sizes_t const & sizes = *sessionState().m_columns_sizes;
			for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
			{
				m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
				//qDebug("sizes: %u %u %u", sizes.at(0), sizes.at(1), sizes.at(2));
			}

			m_table_view_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
			m_table_view_widget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

			connect(m_table_view_widget, SIGNAL(clicked(QModelIndex const &)), this, SLOT(onTableClicked(QModelIndex const &)));
			connect(m_table_view_widget, SIGNAL(doubleClicked(QModelIndex const &)), this, SLOT(onTableDoubleClicked(QModelIndex const &)));
			m_table_view_widget->setContextMenuPolicy(Qt::CustomContextMenu);
			connect(m_table_view_widget, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

			m_table_view_widget->setVisible(true);
			m_table_view_widget->setItemDelegate(new TableItemDelegate(sessionState(), this));

			m_main_window->getTabTrace()->setCurrentIndex(tab_idx);

			//m_table_view_widget->horizontalHeader()->setStretchLastSection(false);
//////////////// PERF!!!!! //////////////////
			// m_table_view_widget->horizontalHeader()->setStretchLastSection(true);
//////////////// PERF!!!!! //////////////////

			static_cast<LogTableModel *>(m_table_view_widget->model())->emitLayoutChanged();
			*/
		}
	}

	//if (m_main_window->statsEnabled())
	//	m_statswindow = new stats::StatsWindow(this, m_session_state);

	qDebug("Server::incomingConnection buffering not enabled, notifying client");
	onBufferingStateChanged(m_main_window->buffState());
	return true;
}

void Connection::setupColumnSizes (bool force_setup)
{
/*	if (force_setup || !m_column_setup_done)
	{
		m_column_setup_done = true;
		bool const old = m_table_view_widget->blockSignals(true);
		{
			m_table_view_widget->horizontalHeader()->resizeSections(QHeaderView::Fixed);

			if (sessionState().m_columns_sizes)
			{
				columns_sizes_t const & sizes = *sessionState().m_columns_sizes;
				for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
				{
					m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
					qDebug("size: col[%i]=%u", c, sizes.at(c));
				}

				m_table_view_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
				m_table_view_widget->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

				columns_setup_t const & global_template = m_main_window->getColumnSetup(sessionState().m_app_idx);
				for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
				{
					if (c >= global_template.size())
					{
						m_table_view_widget->horizontalHeader()->hideSection(c);
					}
				}
			}
		}
		m_table_view_widget->blockSignals(old);
	}*/
	//m_main_window->getWidgetFile()->resizeColumnToContents(true);
}


