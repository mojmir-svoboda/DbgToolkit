#include "connection.h"
#include <QStandardItemModel>
#include <QListView>
#include <QHeaderView>
#include <tlv_parser/tlv_encoder.h>
#include "modelview.h"
#include "utils.h"
#include "utils_qstandarditem.h"
#include "serialization.h"
#include "constants.h"
#include "statswindow.h"
#include "delegates.h"

void Connection::handleCSVSetup (QString const & fname)
{
	qDebug("Connection::handleCSVSetup() this=0x%08x", this);

	this->setupModelColorRegex();
	this->setupModelRegex();
	this->setupModelString();

	QString app_name = fname;
	if (m_main_window->reuseTabEnabled())
	{
		Server * server = static_cast<Server *>(parent());
		Connection * conn = server->findConnectionByName(app_name);
		if (conn)
		{
			if (!m_main_window->clrFltEnabled())
			{
				loadSessionState(conn->sessionState(), m_session_state);
			}

			QWidget * w = conn->sessionState().m_tab_widget;
			server->onCloseTab(w);	// close old one
			// @TODO: delete persistent storage for the tab
		}
		else
		{
			QString const pname = getPresetPath(app_name, g_defaultPresetName);
			m_main_window->onPresetActivate(this, pname);
		}

		{
			QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetColorRegex()->model());
			QStandardItem * root = model->invisibleRootItem();
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

		{
			QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetRegex()->model());
			QStandardItem * root = model->invisibleRootItem();
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
		{
			QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetString()->model());
			QStandardItem * root = model->invisibleRootItem();
			for (int i = 0; i < sessionState().m_filtered_strings.size(); ++i)
			{
				FilteredString & flt = sessionState().m_filtered_strings[i];
				appendToStringWidgets(flt);
			}
		}

	}

	sessionState().m_app_name = app_name;
	//sessionState().m_pid = pid;

	m_table_view_widget->setVisible(false);
	int const tab_idx = m_main_window->getTabTrace()->indexOf(sessionState().m_tab_widget);
	m_main_window->getTabTrace()->setTabText(tab_idx, app_name);

	sessionState().m_app_idx = m_main_window->findAppName(app_name);
	if (sessionState().m_app_idx == e_InvalidItem)
	{
		qDebug("Unknown application: requesting user input");
		sessionState().m_app_idx = m_main_window->createAppName(app_name, e_Proto_CSV);
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
	static_cast<ModelView *>(m_table_view_widget->model())->emitLayoutChanged();

	qDebug("Server::incomingConnection buffering not enabled, notifying client\n");
	onBufferingStateChanged(m_main_window->buffState());
}

void Connection::tryLoadMatchingPreset (QString const & app_name)
{
	m_file_model->beforeLoad();
	QString const preset_name = m_main_window->matchClosestPresetName(app_name);
	m_main_window->onPresetActivate(this, preset_name);
	m_file_model->afterLoad();
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
			this->setupModelFile();
			this->setupModelLvl();
			this->setupModelCtx();
			this->setupModelTID();
			this->setupModelColorRegex();
			this->setupModelRegex();
			this->setupModelString();

			QString app_name = cmd.tvs[i].m_val;
			if (m_main_window->reuseTabEnabled())
			{
				Server * server = static_cast<Server *>(parent());
				Connection * conn = server->findConnectionByName(app_name);
				if (conn)
				{
					if (!m_main_window->clrFltEnabled())
					{
						m_file_model->beforeLoad();
						loadSessionState(conn->sessionState(), m_session_state);
					}

					QWidget * w = conn->sessionState().m_tab_widget;
					server->onCloseTab(w);	// close old one
					// @TODO: delete persistent storage for the tab

					m_file_model->afterLoad();
				}
				else
				{
					m_file_model->beforeLoad();
					QString const pname = getPresetPath(app_name, g_defaultPresetName);
					m_main_window->onPresetActivate(this, pname);
					m_file_model->afterLoad();
				}


				{
					QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetColorRegex()->model());
					QStandardItem * root = model->invisibleRootItem();
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

				{
					QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetRegex()->model());
					QStandardItem * root = model->invisibleRootItem();
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

				{
					QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetLvl()->model());
					QStandardItem * root = model->invisibleRootItem();
					for (int i = 0; i < sessionState().m_lvl_filters.size(); ++i)
					{
						FilteredLevel & flt = sessionState().m_lvl_filters[i];
						appendToLvlWidgets(flt);
					}
				}
				{
					QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetCtx()->model());
					QStandardItem * root = model->invisibleRootItem();
					for (int i = 0; i < sessionState().m_ctx_filters.size(); ++i)
					{
						FilteredContext & flt = sessionState().m_ctx_filters[i];
						appendToCtxWidgets(flt);
					}
				}
				{
					QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetString()->model());
					QStandardItem * root = model->invisibleRootItem();
					for (int i = 0; i < sessionState().m_filtered_strings.size(); ++i)
					{
						FilteredString & flt = sessionState().m_filtered_strings[i];
						appendToStringWidgets(flt);
					}
				}

			}
			else
			{
				tryLoadMatchingPreset(app_name);
			}

			this->setupModelFile();
			this->setupModelLvl();

			sessionState().m_app_name = app_name;
			sessionState().m_pid = pid;

			m_table_view_widget->setVisible(false);
			int const tab_idx = m_main_window->getTabTrace()->indexOf(sessionState().m_tab_widget);
			m_main_window->getTabTrace()->setTabText(tab_idx, app_name);
			QString storage_name = createStorageName();
			setupStorage(storage_name);

			sessionState().m_app_idx = m_main_window->findAppName(app_name);
			if (sessionState().m_app_idx == e_InvalidItem)
			{
				sessionState().m_app_idx = m_main_window->createAppName(app_name, e_Proto_TLV);
			}

			columns_setup_t & cs_setup = m_main_window->getColumnSetup(sessionState().m_app_idx);
			columns_sizes_t & cs_sizes = m_main_window->getColumnSizes(sessionState().m_app_idx);
			columns_align_t & cs_align = m_main_window->getColumnAlign(sessionState().m_app_idx);
			columns_elide_t & cs_elide = m_main_window->getColumnElide(sessionState().m_app_idx);

			if (cs_setup.empty() || cs_sizes.empty() || cs_align.empty() || cs_elide.empty())
			{
				m_main_window->onSetup(e_Proto_TLV, sessionState().m_app_idx, true, true);
			}

			sessionState().setupColumns(&cs_setup, &cs_sizes, &cs_align, &cs_elide); 

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
			/* m_table_view_widget->horizontalHeader()->setStretchLastSection(true); */
//////////////// PERF!!!!! //////////////////

			static_cast<ModelView *>(m_table_view_widget->model())->emitLayoutChanged();
		}
	}

	if (m_main_window->statsEnabled())
		m_statswindow = new stats::StatsWindow(this, m_session_state);

	qDebug("Server::incomingConnection buffering not enabled, notifying client\n");
	onBufferingStateChanged(m_main_window->buffState());
	return true;
}

void Connection::setupColumnSizes (bool force_setup)
{
	if (force_setup || !m_column_setup_done)
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
	}
	//m_main_window->getWidgetFile()->resizeColumnToContents(true);
}

void Connection::setupModelFile ()
{
	if (!m_file_model)
	{
		qDebug("new tree view file model");
		m_file_model = new TreeModel(this, &m_session_state.m_file_filters);
	}
	m_main_window->getWidgetFile()->setModel(m_file_model);
	m_main_window->getWidgetFile()->syncExpandState();
	m_main_window->getWidgetFile()->hideLinearParents();
	connect(m_file_model, SIGNAL(invalidateFilter()), this, SLOT(onInvalidateFilter()));
}

void Connection::destroyModelFile ()
{
	if (m_file_model)
	{
		qDebug("destroying file model");
		disconnect(m_file_model, SIGNAL(invalidateFilter()), this, SLOT(onInvalidateFilter()));
		m_main_window->getWidgetFile()->unsetModel(m_file_model);
		delete m_file_model;
		m_file_model = 0;
	}
}

void Connection::setupModelCtx ()
{
	if (!m_ctx_model)
	{
		qDebug("new tree view ctx model");
		m_ctx_model = new QStandardItemModel;
	}
	m_main_window->getWidgetCtx()->setModel(m_ctx_model);
	m_main_window->getWidgetCtx()->expandAll();
	m_main_window->getWidgetCtx()->setItemDelegate(m_ctx_delegate);
}

void Connection::setupModelTID ()
{
	if (!m_tid_model)
		m_tid_model = new QStandardItemModel;
	m_main_window->getWidgetTID()->setModel(m_tid_model);
}

void Connection::setupModelColorRegex ()
{
	if (!m_color_regex_model)
		m_color_regex_model = new QStandardItemModel;
	m_main_window->getWidgetColorRegex()->setModel(m_color_regex_model);
}

void Connection::setupModelRegex ()
{
	if (!m_regex_model)
		m_regex_model = new QStandardItemModel;
	m_main_window->getWidgetRegex()->setModel(m_regex_model);
}

void Connection::setupModelString ()
{
	if (!m_string_model)
		m_string_model = new QStandardItemModel;
	m_main_window->getWidgetString()->setModel(m_string_model);
}

void Connection::setupModelLvl ()
{
	if (!m_lvl_model)
		m_lvl_model = new QStandardItemModel;
	m_main_window->getWidgetLvl()->setModel(m_lvl_model);
	m_main_window->getWidgetLvl()->setSortingEnabled(true);
	m_main_window->getWidgetLvl()->setItemDelegate(m_lvl_delegate);
	m_main_window->getWidgetLvl()->setRootIndex(m_lvl_model->indexFromItem(m_lvl_model->invisibleRootItem()));
}
