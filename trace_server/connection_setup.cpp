#include "connection.h"
#include <QStandardItemModel>
#include <QListView>
#include <tlv_parser/tlv_encoder.h>
#include "modelview.h"

bool Connection::handleSetupCommand (DecodedCommand const & cmd)
{
	qDebug("handle setup command");
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_lvl)
		{
			int const level = QString::fromStdString(cmd.tvs[i].m_val).toInt();
            m_main_window->setLevel(level);
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
					QWidget * w = conn->sessionState().m_tab_widget;
					server->onCloseTab(w);	// close old one
					// @TODO: delete persistent storage for the tab
					sessionState().m_tab_idx = m_main_window->getTabTrace()->indexOf(sessionState().m_tab_widget);
					
					this->setupModelFile();
					this->setupModelTID();
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
		m_tree_view_file_model = new QStandardItemModel;
	m_main_window->getTreeViewFile()->setModel(m_tree_view_file_model);
	//main_window->getTreeViewFile()->expandAll();
	m_main_window->getTreeViewFile()->setEnabled(m_main_window->filterEnabled());
}

void Connection::setupModelTID ()
{
	if (!m_list_view_tid_model)
		m_list_view_tid_model = new QStandardItemModel;
	m_main_window->getListViewTID()->setModel(m_list_view_tid_model);
}

void Connection::setupModelColorRegex ()
{
	if (!m_list_view_color_regex_model)
		m_list_view_color_regex_model = new QStandardItemModel;
	m_main_window->getListViewColorRegex()->setModel(m_list_view_color_regex_model);
}

