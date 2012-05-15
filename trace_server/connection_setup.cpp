#include "connection.h"
#include <QStandardItemModel>
#include <QListView>
#include <tlv_parser/tlv_encoder.h>
#include "modelview.h"

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
					if (!m_main_window->clrFltEnabled())
					{
						boost::char_separator<char> sep(":/\\");
						SessionExport e;
						conn->sessionState().sessionExport(e);
						sessionState().sessionImport(e);

						boost::char_separator<char> sep2("\n");
						typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
						tokenizer_t tok(e.m_file_filters, sep2);
						for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
						{
							//appendToFileFilters(sep, *it, true);
							loadToFileFilters(*it);
						}
					}

					m_main_window->getTreeViewFile()->expandAll();
					m_main_window->getTreeViewFile()->setEnabled(m_main_window->filterEnabled());
	
					QWidget * w = conn->sessionState().m_tab_widget;
					server->onCloseTab(w);	// close old one
					// @TODO: delete persistent storage for the tab
					sessionState().m_tab_idx = m_main_window->getTabTrace()->indexOf(sessionState().m_tab_widget);
					onTabTraceFocus(sessionState().m_tab_idx);
				}
			}

			sessionState().m_name = app_name;

			m_main_window->getTabTrace()->setTabText(sessionState().m_tab_idx, app_name);
			QString storage_name = createStorageName();
			setupStorage(storage_name);

			sessionState().m_app_idx = m_main_window->findAppName(app_name);
			sessionState().setupColumns(&m_main_window->getColumnSetup(sessionState().m_app_idx), &m_main_window->getColumnSizes(sessionState().m_app_idx));

			m_current_cmd.tvs.reserve(sessionState().getColumnsSetupCurrent()->size());

			for (size_t i = 0, ie = sessionState().getColumnsSetupCurrent()->size(); i < ie; ++i)
			{
				m_table_view_widget->model()->insertColumn(i);
			}

			/*MainWindow::columns_sizes_t const & sizes = *sessionState().m_columns_sizes;
			for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
			{
				m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
			}*/
			connect(m_table_view_widget, SIGNAL(clicked(QModelIndex const &)), this, SLOT(onTableClicked(QModelIndex const &)));
			connect(m_table_view_widget, SIGNAL(doubleClicked(QModelIndex const &)), this, SLOT(onTableDoubleClicked(QModelIndex const &)));
			m_table_view_widget->setContextMenuPolicy(Qt::CustomContextMenu);
			connect(m_table_view_widget, SIGNAL(customContextMenuRequested(QPoint const &)), this, SLOT(onShowContextMenu(QPoint const &)));

			static_cast<ModelView *>(m_table_view_widget->model())->emitLayoutChanged();
		}
	}

	if (!m_main_window->buffEnabled())
	{
		qDebug("Server::incomingConnection buffering not enabled, notifying client\n");
		onBufferingStateChanged(m_main_window->buffEnabled());
	}
	return true;
}

void Connection::setupModelFile ()
{
	if (!m_tree_view_file_model)
	{
		qDebug("new tree view file model");
		m_tree_view_file_model = new QStandardItemModel;
	}
	m_main_window->getTreeViewFile()->setModel(m_tree_view_file_model);
	m_main_window->getTreeViewFile()->expandAll();
	m_main_window->getTreeViewFile()->setEnabled(m_main_window->filterEnabled());
}

void Connection::setupModelCtx ()
{
	if (!m_tree_view_ctx_model)
	{
		qDebug("new tree view ctx model");
		m_tree_view_ctx_model = new QStandardItemModel;
	}
	m_main_window->getTreeViewCtx()->setModel(m_tree_view_ctx_model);
	m_main_window->getTreeViewCtx()->expandAll();
	m_main_window->getTreeViewCtx()->setEnabled(m_main_window->filterEnabled());
}

void Connection::setupModelTID ()
{
	if (!m_list_view_tid_model)
		m_list_view_tid_model = new QStandardItemModel;
	m_main_window->getListViewTID()->setModel(m_list_view_tid_model);
	m_main_window->getListViewTID()->setEnabled(m_main_window->filterEnabled());
}

void Connection::setupModelColorRegex ()
{
	if (!m_list_view_color_regex_model)
		m_list_view_color_regex_model = new QStandardItemModel;
	m_main_window->getListViewColorRegex()->setModel(m_list_view_color_regex_model);
	m_main_window->getListViewColorRegex()->setEnabled(m_main_window->filterEnabled());
}

