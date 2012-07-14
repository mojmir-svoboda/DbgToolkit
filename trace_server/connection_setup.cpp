#include "connection.h"
#include <QStandardItemModel>
#include <QListView>
#include <QHeaderView>
#include <tlv_parser/tlv_encoder.h>
#include "modelview.h"
#include "utils.h"
#include "serialization.h"
#include "constants.h"
#include "statswindow.h"

bool Connection::handleSetupCommand (DecodedCommand const & cmd)
{
	qDebug("Connection::handleSetupCommand() this=0x%08x", this);

	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_lvl)
		{
			int const client_level = QString::fromStdString(cmd.tvs[i].m_val).toInt();
			int const server_level = m_main_window->getLevel();
			if (client_level != server_level)
			{
				qDebug("notifying client about new level");
				onLevelValueChanged(server_level);
			}
		}

		if (cmd.tvs[i].m_tag == tlv::tag_app)
		{
			QString app_name = QString::fromStdString(cmd.tvs[i].m_val);
			if (m_main_window->reuseTabEnabled())
			{
				Server * server = static_cast<Server *>(parent());
				Connection * conn = server->findConnectionByName(app_name);
				if (conn)
				{
					this->setupModelFile();
					this->setupModelCtx();
					this->setupModelTID();
					this->setupModelLvl();
					this->setupModelColorRegex();
					this->setupModelRegex();
					if (!m_main_window->clrFltEnabled())
					{
						loadSessionState(conn->sessionState(), m_session_state);

						{
							QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetColorRegex()->model());
							QStandardItem * root = model->invisibleRootItem();
							for (int i = 0; i < sessionState().m_colorized_texts.size(); ++i)
							{
								ColorizedText & ct = sessionState().m_colorized_texts[i];
								ct.m_regex = QRegExp(QString::fromStdString(ct.m_regex_str));

								QStandardItem * child = findChildByText(root, QString::fromStdString(ct.m_regex_str));
								if (child == 0)
								{
									QList<QStandardItem *> row_items = addRow(QString::fromStdString(ct.m_regex_str), ct.m_is_enabled);
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
								flt.m_regex = QRegExp(QString::fromStdString(flt.m_regex_str));

								QStandardItem * child = findChildByText(root, QString::fromStdString(flt.m_regex_str));
								if (child == 0)
								{
									QList<QStandardItem *> row_items = addTriRow(QString::fromStdString(flt.m_regex_str), flt.m_is_enabled ? Qt::Checked : Qt::Unchecked, flt.m_is_inclusive);
									root->appendRow(row_items);
									child = findChildByText(root, QString::fromStdString(flt.m_regex_str));
									child->setCheckState(Qt::Checked);
								}
							}
							recompileRegexps();
						}

						{
							QStandardItemModel * model = static_cast<QStandardItemModel *>(m_main_window->getWidgetRegex()->model());
							QStandardItem * root = model->invisibleRootItem();
							for (int i = 0; i < sessionState().m_filtered_regexps.size(); ++i)
							{
								FilteredRegex & flt = sessionState().m_filtered_regexps[i];
								flt.m_regex = QRegExp(QString::fromStdString(flt.m_regex_str));

								QStandardItem * child = findChildByText(root, QString::fromStdString(flt.m_regex_str));
								if (child == 0)
								{
									QList<QStandardItem *> row_items = addTriRow(QString::fromStdString(flt.m_regex_str), flt.m_is_enabled ? Qt::Checked : Qt::Unchecked, flt.m_is_inclusive);
									root->appendRow(row_items);
									child = findChildByText(root, QString::fromStdString(flt.m_regex_str));
									child->setCheckState(Qt::Checked);
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
					}

					m_main_window->getWidgetFile()->setEnabled(m_main_window->filterEnabled());
	
					QWidget * w = conn->sessionState().m_tab_widget;
					server->onCloseTab(w);	// close old one
					// @TODO: delete persistent storage for the tab
					sessionState().m_tab_idx = m_main_window->getTabTrace()->indexOf(sessionState().m_tab_widget);
					onTabTraceFocus(sessionState().m_tab_idx);
				}
				else
				{
					QString const pname = getPresetPath(app_name, g_defaultPresetName);
					QString const total_path = getPresetFileName(m_main_window->getAppDir(), pname);
					if (existsPresetFile(total_path.toAscii()))
						m_main_window->loadSession(m_session_state, pname);

					m_main_window->onPresetActivate(this, pname);
					m_main_window->setPresetNameIntoComboBox(pname);
				}
				
			}

			sessionState().m_name = app_name;

			m_table_view_widget->setVisible(false);
			m_main_window->getTabTrace()->setTabText(sessionState().m_tab_idx, app_name);
			QString storage_name = createStorageName();
			setupStorage(storage_name);

			sessionState().m_app_idx = m_main_window->findAppName(app_name);
			sessionState().setupColumns(&m_main_window->getColumnSetup(sessionState().m_app_idx), &m_main_window->getColumnSizes(sessionState().m_app_idx)
					, &m_main_window->getColumnAlign(sessionState().m_app_idx)
					, &m_main_window->getColumnElide(sessionState().m_app_idx));

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

			m_table_view_widget->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
			m_table_view_widget->verticalHeader()->setResizeMode(QHeaderView::Fixed);

			connect(m_table_view_widget, SIGNAL(clicked(QModelIndex const &)), this, SLOT(onTableClicked(QModelIndex const &)));
			connect(m_table_view_widget, SIGNAL(doubleClicked(QModelIndex const &)), this, SLOT(onTableDoubleClicked(QModelIndex const &)));
			m_table_view_widget->setContextMenuPolicy(Qt::CustomContextMenu);
			connect(m_table_view_widget, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

			m_table_view_widget->setVisible(true);

			//m_table_view_widget->horizontalHeader()->setStretchLastSection(false);
//////////////// PERF!!!!! //////////////////
	/*
			m_table_view_widget->horizontalHeader()->setStretchLastSection(true);

	*/
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

void Connection::setupColumnSizes ()
{
	if (!m_column_setup_done)
	{
		m_column_setup_done = true;
		bool const old = m_table_view_widget->blockSignals(true);
		{
			m_table_view_widget->horizontalHeader()->resizeSections(QHeaderView::Fixed);

			columns_sizes_t const & sizes = *sessionState().m_columns_sizes;
			for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
			{
				m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
				qDebug("size: col[%i]=%u", c, sizes.at(c));
			}

			m_table_view_widget->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
			m_table_view_widget->verticalHeader()->setResizeMode(QHeaderView::Interactive);

			columns_setup_t const & global_template = m_main_window->getColumnSetup(sessionState().m_app_idx);
			for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
			{
				if (c >= global_template.size())
				{
					m_table_view_widget->horizontalHeader()->hideSection(c);
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
		m_file_model = new QStandardItemModel;
	}
	m_main_window->getWidgetFile()->setModel(m_file_model);
	m_main_window->getWidgetFile()->expandAll();
	m_main_window->getWidgetFile()->setEnabled(m_main_window->filterEnabled());
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
	m_main_window->getWidgetCtx()->setEnabled(m_main_window->filterEnabled());
}

void Connection::setupModelTID ()
{
	if (!m_tid_model)
		m_tid_model = new QStandardItemModel;
	m_main_window->getWidgetTID()->setModel(m_tid_model);
	m_main_window->getWidgetTID()->setEnabled(m_main_window->filterEnabled());
}

void Connection::setupModelColorRegex ()
{
	if (!m_color_regex_model)
		m_color_regex_model = new QStandardItemModel;
	m_main_window->getWidgetColorRegex()->setModel(m_color_regex_model);
	m_main_window->getWidgetColorRegex()->setEnabled(m_main_window->filterEnabled());
}

void Connection::setupModelRegex ()
{
	if (!m_regex_model)
		m_regex_model = new QStandardItemModel;
	m_main_window->getWidgetRegex()->setModel(m_regex_model);
	m_main_window->getWidgetRegex()->setEnabled(m_main_window->filterEnabled());
}

void Connection::setupModelLvl ()
{
	if (!m_lvl_model)
		m_lvl_model = new QStandardItemModel;
	m_main_window->getWidgetLvl()->setModel(m_lvl_model);
	m_main_window->getWidgetLvl()->setEnabled(m_main_window->filterEnabled());
	m_main_window->getWidgetLvl()->setSortingEnabled(true);
	m_main_window->getWidgetLvl()->setItemDelegate(new LevelDelegate(sessionState(), this));
}
