#include "connection.h"
#include <QtNetwork>
#include <QCheckBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QFile>
#include <QDataStream>
#include <QTime>
#include <QTimer>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QListView>
#include <boost/tokenizer.hpp>
#include <tlv_parser/tlv_encoder.h>
#include <trace_client/trace.h>
#include "modelview.h"

Connection::Connection (QObject * parent)
	: QThread(parent)
	, m_main_window(0)
	, m_from_file(false)
	, m_first_line(true)
	, m_last_search_row(0)
	, m_last_search_col(0)
	, m_table_view_widget(0)
	, m_tree_view_file_model(0)
	, m_tree_view_func_model(0)
	, m_list_view_tid_model(0)
	, m_table_view_proxy(0)
	, m_buffer(e_ringbuff_size)
	, m_current_cmd()
	, m_decoded_cmds(e_ringcmd_size)
	, m_decoder()
	, m_storage(0)
	, m_datastream(0)
	, m_tcpstream(0)
{ }

Connection::~Connection () { qDebug("~Connection()"); }

void Connection::onDisconnected ()
{
	qDebug("onDisconnected()");
	closeStorage();
}

void Connection::onTabTraceFocus (int i)
{
	if (i != sessionState().m_tab_idx)
		return;
	m_main_window->getTreeViewFile()->setModel(m_tree_view_file_model);
	m_main_window->getListViewTID()->setModel(m_list_view_tid_model);
	hideLinearParents();
}

void Connection::hideLinearParents ()
{
	QStandardItem * node = m_tree_view_file_model->invisibleRootItem();
	QStandardItem * last_hidden_node = 0;
	while (node)
	{
		QStandardItem * child = node->child(0);
		if (child != 0)
		{
			if (child->rowCount() == 1)
				last_hidden_node = child;
			else if (child->rowCount() > 1)
			{
				last_hidden_node = child;
				break;
			}
		}
		else
			break;

		node = child;
	}
	if (last_hidden_node)
	{
		m_main_window->getTreeViewFile()->setRootIndex(last_hidden_node->index());
	}
}

void Connection::onCloseTab ()
{
	qDebug("onCloseTab: this=%08x", this);
	if (m_tcpstream)
	{
		QObject::disconnect(m_tcpstream, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
		QObject::disconnect(m_tcpstream, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	}
	if (m_tcpstream)
		m_tcpstream->close();
	closeStorage();
	if (m_main_window->getTreeViewFile()->model() == m_tree_view_file_model)
		m_main_window->getTreeViewFile()->setModel(0);

	if (m_main_window->getListViewTID()->model() == m_list_view_tid_model)
		m_main_window->getListViewTID()->setModel(0);

	delete m_tree_view_file_model;
	m_table_view_widget = 0;
	delete m_list_view_tid_model;
	m_list_view_tid_model = 0;
}

void Connection::onLevelValueChanged (int val)
{
	char tlv_buff[16];
#ifdef __linux__
	int const result = snprintf(tlv_buff, 16, "%u", val);
#else
	int const result = _snprintf(tlv_buff, 16, "%u", val);
#endif

	if (result > 0)
	{
		char buff[256];
		using namespace tlv;
		Encoder e(cmd_set_level, buff, 256);
		e.Encode(TLV(tag_lvl, tlv_buff));
		if (m_tcpstream && e.Commit())
			m_tcpstream->write(e.buffer, e.total_len); /// @TODO: async write
	}
}

void Connection::onBufferingStateChanged (int val)
{
	char tlv_buff[16];
#ifdef __linux__
	int const result = snprintf(tlv_buff, 16, "%u", val);
#else
	int const result = _snprintf(tlv_buff, 16, "%u", val);
#endif

	if (result > 0)
	{
		qDebug("Connection::onBufferingStateChanged to_state=%i", val);
		char buff[256];
		using namespace tlv;
		Encoder e(cmd_set_buffering, buff, 256);
		e.Encode(TLV(tag_bool, tlv_buff));
		if (m_tcpstream && e.Commit())
			m_tcpstream->write(e.buffer, e.total_len); /// @TODO: async write
	}
}

QString Connection::onCopyToClipboard ()
{
	QAbstractItemModel * model = m_table_view_widget->model();
	QItemSelectionModel * selection = m_table_view_widget->selectionModel();
	QModelIndexList indexes = selection->selectedIndexes();

	if (indexes.size() < 1)
		return QString();

	std::sort(indexes.begin(), indexes.end());

	QString selected_text;
	selected_text.reserve(4096);
	for (int i = 0; i < indexes.size(); ++i)
	{
		QModelIndex const current = indexes.at(i);
		selected_text.append(model->data(current).toString());
		
		if (i + 1 < indexes.size() && current.row() != indexes.at(i + 1).row())
			selected_text.append('\n');	// switching rows
		else
			selected_text.append('\t');
	}
	return selected_text;
}


void Connection::findTextInAllColumns (QString const & text, int from_row, int to_row)
{
	ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	for (int i = from_row, ie = to_row; i < ie; ++i)
	{
		for (int j = 0, je = model->columnCount(); j < je; ++j)
		{
			QModelIndex const idx = model->index(i, j, QModelIndex());
			if (idx.isValid() && model->data(idx).toString().contains(text))
			{
				m_table_view_widget->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Select);
				m_last_search_row = idx.row();
				m_last_search_col = idx.column();
				return;
			}
		}
	}
}

void Connection::findTextInColumn (QString const & text, int col, int from_row, int to_row)
{
	ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	for (int i = from_row, ie = to_row; i < ie; ++i)
	{
		QModelIndex const idx = model->index(i, col, QModelIndex());
		if (idx.isValid() && model->data(idx).toString().contains(text))
		{
			m_table_view_widget->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::Select);
			m_last_search_row = idx.row();

            m_table_view_widget->scrollTo(m_table_view_proxy ? m_table_view_proxy->mapFromSource(idx) : idx, QTableView::PositionAtCenter);
			return;
		}
	}
	{
		qDebug("end of search");
		// flash icon
		m_last_search_row = 0;
	}
}

void Connection::selectionFromTo (int & from, int & to) const
{
	from = 0;
	ModelView const * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	to = model->rowCount();
	QItemSelectionModel const * selection = m_table_view_widget->selectionModel();
	QModelIndexList indexes = selection->selectedIndexes();
	if (indexes.size() < 1)
		return;

	std::sort(indexes.begin(), indexes.end());

	from = indexes.first().row();
}

void Connection::findText (QString const & text, tlv::tag_t tag)
{
	if (m_last_search != text)
	{
		m_last_search_row = 0;	// this is a new search
		m_last_search = text;
		int const col_idx = sessionState().findColumn4Tag(tag);
		m_last_search_col = col_idx;

		if (m_last_search.isEmpty())
		{
			m_last_search_row = m_last_search_col = 0;
			return;
		}


		//@TODO: clear selection?
		int from, to;
		selectionFromTo(from, to);
		findTextInColumn(m_last_search, col_idx, from, to);
	}
	else
	{
		ModelView const * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
		int const to = model->rowCount();
		findTextInColumn(m_last_search, m_last_search_col, m_last_search_row + 1, to);
	}
}

void Connection::findText (QString const & text)
{
	m_last_search = text;
	m_last_search_row = 0;
	m_last_search_col = -1;

	if (m_last_search.isEmpty())
	{
		m_last_search_row = m_last_search_col = 0;
		return;
	}

	int from, to;
	selectionFromTo(from, to);
	findTextInAllColumns(m_last_search, from, to);
}

void Connection::findNext ()
{
	int from, to;
	selectionFromTo(from, to);
	findTextInColumn(m_last_search, m_last_search_col, m_last_search_row + 1, to);
}

void Connection::findPrev ()
{
}

//////////////////// data reading stuff //////////////////////////////
inline size_t read_min (boost::circular_buffer<char> & ring, char * dst, size_t min)
{
	if (ring.size() < min)
		return 0;

	for (size_t i = 0; i < min; ++i)
	{
		dst[i] = ring.front();
		ring.pop_front();
	}
	return min;
}

void Connection::onHandleCommands ()
{
	ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	size_t const rows = m_decoded_cmds.size();
	model->transactionStart(rows);
	for (size_t i = 0; i < rows; ++i)
	{
		DecodedCommand & cmd = m_decoded_cmds.front();
		tryHandleCommand(cmd);
		m_decoded_cmds.pop_front();
	}

		// sigh. hotfix for disobedient column resizing @TODO: resolve in future
	//if (m_first_line)	// resize columns according to saved template
	{
		MainWindow::columns_sizes_t const & sizes = m_main_window->getColumnSizes(sessionState().m_app_idx);
		MainWindow::columns_setup_t const & global_template = m_main_window->getColumnSetup(sessionState().m_app_idx);

		if (global_template.empty())
		{
			Q_ASSERT(sessionState().getColumnsSetupCurrent());
			m_main_window->getColumnSetup(sessionState().m_app_idx) = *sessionState().getColumnsSetupCurrent();
		}
		
		bool const old = m_table_view_widget->blockSignals(true);
		for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
			m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
		m_table_view_widget->blockSignals(old);
		m_first_line = false;
	}

	{
		// hotfix for disobedient column hiding @TODO: resolve in future
		MainWindow::columns_sizes_t const & sizes = m_main_window->getColumnSizes(sessionState().m_app_idx);
		MainWindow::columns_setup_t const & global_template = m_main_window->getColumnSetup(sessionState().m_app_idx);
		for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
		{
			if (c >= global_template.size())
			{
				m_table_view_widget->horizontalHeader()->hideSection(c);
				//m_table_view_widget->hideColumn(c);
			}
		}
		//static_cast<ModelView *>(m_table_view_widget->model())->emitLayoutChanged();
		//m_main_window->getTreeViewFile()->setResizeMode(relevantColumn, QHeaderView::ResizeToContents);
		m_main_window->getTreeViewFile()->resizeColumnToContents(0);
	}

	model->transactionCommit();

	if (m_main_window->autoScrollEnabled())
		m_table_view_widget->scrollToBottom();
}

template <class T, typename T_Ret, typename T_Arg0, typename T_Arg1>
void Connection::processStream (T * t, T_Ret (T::*read_member_fn)(T_Arg0, T_Arg1))
{
	enum { local_buff_sz = 128 };
	char local_buff[local_buff_sz];
	try {

		// read data into ring buffer
		while (!m_buffer.full())
		{
			size_t const free_space = m_buffer.reserve();
			size_t const to_read = free_space < local_buff_sz ? free_space : local_buff_sz;

			qint64 const count = (t->*read_member_fn)(local_buff, to_read);
			if (count <= 0)
				break;	// no more data in stream

			for (size_t i = 0; i < count; ++i)
				m_buffer.push_back(local_buff[i]);
		}

		// try process data in ring buffer
		while (!m_decoded_cmds.full())
		{
			if (!m_current_cmd.written_hdr)
			{
				size_t const count_hdr = read_min(m_buffer, &m_current_cmd.orig_message[0], tlv::Header::e_Size);
				if (count_hdr == tlv::Header::e_Size)
				{
					m_decoder.decode_header(&m_current_cmd.orig_message[0], tlv::Header::e_Size, m_current_cmd);
					if (m_current_cmd.hdr.cmd == 0 || m_current_cmd.hdr.len == 0)
					{
						Q_ASSERT(false);
						//@TODO: parsing error, discard everything and re-request stop sequence
						break;
					}
					m_current_cmd.written_hdr = true;
				}
				else
					break; // not enough data
			}

			if (m_current_cmd.written_hdr && !m_current_cmd.written_payload)
			{
				size_t const count_payload = read_min(m_buffer, &m_current_cmd.orig_message[0] + tlv::Header::e_Size, m_current_cmd.hdr.len);
				if (count_payload == m_current_cmd.hdr.len)
					m_current_cmd.written_payload = true;
				else
					break; // not enough data
			}

			if (m_current_cmd.written_hdr && m_current_cmd.written_payload)
			{
				if (m_decoder.decode_payload(&m_current_cmd.orig_message[0] + tlv::Header::e_Size, m_current_cmd.hdr.len, m_current_cmd))
				{
					//qDebug("CONN: hdr_sz=%u payload_sz=%u buff_sz=%u ",  tlv::Header::e_Size, m_current_cmd.hdr.len, m_buffer.size());
					m_decoded_cmds.push_back(m_current_cmd);
				}
				else
				{
					Q_ASSERT(false);
					//@TODO: parsing error, discard everything and re-request stop sequence
					break;
				}

				m_current_cmd.Reset(); // reset current command for another decoding pass
			}
		}

		if (m_decoded_cmds.size() > 0)
			emit handleCommands();
	}
	catch (std::out_of_range const & e)
	{
		QMessageBox::critical(0, tr("My Application"),
								tr("OOR exception during decoding: %1").arg(e.what()),
								QMessageBox::Ok, QMessageBox::Ok);	
	}
	catch (std::length_error const & e)
	{
		QMessageBox::critical(0, tr("My Application"),
								tr("LE exception during decoding: %1").arg(e.what()),
								QMessageBox::Ok, QMessageBox::Ok);
	}
	catch (std::exception const & e)
	{
		QMessageBox::critical(0, tr("My Application"),
								tr("generic exception during decoding: %1").arg(e.what()),
								QMessageBox::Ok, QMessageBox::Ok);
	}
	catch (...)
	{
		QMessageBox::critical(0, tr("My Application"),
								tr("... exception during decoding"),
								QMessageBox::Ok, QMessageBox::Ok);
	}
}

void Connection::processReadyRead ()
{
	processStream(static_cast<QIODevice *>(m_tcpstream), &QTcpSocket::read);
}

void Connection::setSocketDescriptor (int sd)
{
	m_tcpstream = new QTcpSocket(this);
	m_tcpstream->setSocketDescriptor(sd);
	connect(this, SIGNAL(handleCommands()), this, SLOT(onHandleCommands()));
}

void Connection::run ()
{
	while (1)
	{
		const int Timeout = 5 * 1000;
		while (m_tcpstream->bytesAvailable() < tlv::Header::e_Size)
			m_tcpstream->waitForReadyRead(Timeout);

		processReadyRead();
	}
}

void Connection::processDataStream (QDataStream & stream)
{
	m_from_file = true;

	connect(this, SIGNAL(handleCommands()), this, SLOT(onHandleCommands()));
	processStream(&stream, &QDataStream::readRawData);
}

bool Connection::tryHandleCommand (DecodedCommand const & cmd)
{		
	if (cmd.hdr.cmd == tlv::cmd_log)
	{
		handleLogCommand(cmd);
	}
	else if (cmd.hdr.cmd == tlv::cmd_setup)
	{
		handleSetupCommand(cmd);
	}
	else if (cmd.hdr.cmd == tlv::cmd_scope_entry)
	{
		handleLogCommand(cmd);	
	}
	else if (cmd.hdr.cmd == tlv::cmd_scope_exit)
	{
		handleLogCommand(cmd);
	}

	if (!m_from_file && m_datastream) // @TODO: && persistenCheckBox == true
		m_datastream->writeRawData(&cmd.orig_message[0], cmd.hdr.len + tlv::Header::e_Size);
	return true;
}

inline QStandardItem * findChildByText (QStandardItem * parent, QString const & txt)
{
	for (size_t i = 0, ie = parent->rowCount(); i < ie; ++i)
	{
		if (parent->child(i)->text() == txt)
			return parent->child(i);
	}
	return 0;
}

inline QString Connection::findString4Tag (tlv::tag_t tag, QModelIndex const & row_index) const
{
	return findVariant4Tag(tag, row_index).toString();
}

QVariant Connection::findVariant4Tag (tlv::tag_t tag, QModelIndex const & row_index) const
{
	int const idx = sessionState().m_tags2columns[tag];
	if (idx == -1)
		return QVariant();

	QModelIndex model_idx = m_table_view_widget->model()->index(row_index.row(), idx, QModelIndex());
	if (model_idx.isValid())
	{
		QVariant value = m_table_view_widget->model()->data(model_idx);
		return value;
	}
	return QVariant();
}

void Connection::onTableClicked (QModelIndex const & row_index)
{
	QString file = findString4Tag(tlv::tag_file, row_index);
	QString line = findString4Tag(tlv::tag_line, row_index);

	boost::char_separator<char> sep(":/\\");
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
	std::string fstr = file.toStdString();
	tokenizer_t tok(fstr, sep);

	QStandardItem * item = m_tree_view_file_model->invisibleRootItem();
	for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
	{
		QString qfile = QString::fromStdString(*it);
		QStandardItem * child = findChildByText(item, qfile);
		if (child != 0)
		{
			item = child;
			QModelIndex idx = item->index();
			m_main_window->getTreeViewFile()->setExpanded(idx, true);
			m_main_window->getTreeViewFile()->setCurrentIndex(idx);
		}
	}

	if (item != 0)
	{
		QStandardItem * last_level = findChildByText(item, line);
		if (last_level != 0)
		{
			QModelIndex idx = last_level->index();
			m_main_window->getTreeViewFile()->setExpanded(idx, true);
			m_main_window->getTreeViewFile()->setCurrentIndex(idx);
		}
	}

	QString tid = findString4Tag(tlv::tag_tid, row_index);
	QModelIndexList indexList = m_list_view_tid_model->match(m_list_view_tid_model->index(0, 0), Qt::DisplayRole, tid);
	QModelIndex selectedIndex(indexList.first());
	m_main_window->getListViewTID()->setCurrentIndex(selectedIndex);


	// set search from this line
	m_last_search_row = row_index.row();
	//m_table_view_widget->scrollTo(m_table_view_proxy ? m_table_view_proxy->mapFromSource(idx) : idx, QTableView::PositionAtCenter);
}

void Connection::onTableDoubleClicked (QModelIndex const & row_index)
{
	ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());

	int row_bgn = row_index.row();
	int row_end = row_index.row();
	int layer = model->layers()[row_bgn];

	if (model->rowTypes()[row_bgn] == tlv::cmd_scope_exit)
	{
		layer += 1;
		// test na range
		--row_bgn;
	}

	QString tid = findString4Tag(tlv::tag_tid, row_index);
	int from = row_bgn;

	if (model->rowTypes()[from] != tlv::cmd_scope_entry)
	{
		while (row_bgn > 0)
		{
			{
				//QModelIndex const curr_idx = model->index(row_bgn, row_index.column(), QModelIndex());
				//QModelIndex const src = m_table_view_proxy->mapFromSource(curr_idx);
				//qDebug("BGN: (%i,col) -> (%i,col)", row_bgn, src.row());
			}

			QModelIndex curr_idx = model->index(row_bgn, row_index.column(), QModelIndex());
			if (m_table_view_proxy)
			{
				curr_idx = m_table_view_proxy->mapFromSource(curr_idx);
			}

			QString curr_tid = findString4Tag(tlv::tag_tid, curr_idx);
			if (curr_tid == tid)
			{
				if (model->layers()[row_bgn] >= layer)
				{
					from = row_bgn;
				}
				else
				{
					break;
				}
			}
			--row_bgn;
		}
	}

	int to = row_end;
	if (model->rowTypes()[to] != tlv::cmd_scope_exit)
	{
		while (row_end < model->layers().size())
		{
			//QModelIndex const curr_idx = model->index(row_end, row_index.column(), QModelIndex());
			//QModelIndex const src = m_table_view_proxy->mapFromSource(curr_idx);
			//qDebug("END: (%i,col) -> (%i,col)", row_end, src.row());
			QModelIndex curr_idx = model->index(row_end, row_index.column(), QModelIndex());
			if (m_table_view_proxy)
			{
				curr_idx = m_table_view_proxy->mapFromSource(curr_idx);
			}
			QString next_tid = findString4Tag(tlv::tag_tid, curr_idx);
			if (next_tid == tid)
			{
				if (model->layers()[row_end] >= layer)
					to = row_end;
				else if ((model->layers()[row_end] == layer - 1) && (model->rowTypes()[row_end] == tlv::cmd_scope_exit))
				{
					to = row_end;
					break;
				}
				else
					break;
			}
			++row_end;
		}
	}

	qDebug("curr_row=%u, layer=%u, from,to=(%u, %u)", row_index.row(), layer, from, to);
	m_session_state.appendCollapsedBlock(tid, from, to);
	onInvalidateFilter();
}

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

bool Connection::handleLogCommand (DecodedCommand const & cmd)
{
	appendToFilters(cmd);

	if (cmd.hdr.cmd == tlv::cmd_scope_entry || (cmd.hdr.cmd == tlv::cmd_scope_exit))
	{
		if (!m_main_window->scopesEnabled())
		{
			ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
			model->appendCommand(m_table_view_proxy, cmd);
		}
	}
	else if (cmd.hdr.cmd == tlv::cmd_log)
	{
		ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
		model->appendCommand(m_table_view_proxy, cmd);
	}
	return true;
}

//////////////////// filtering stuff //////////////////////////////
FilterProxyModel::FilterProxyModel (QObject * parent, QList<QRegExp> const & r, std::vector<bool> const & rs, SessionState & ss)
	: QSortFilterProxyModel(parent), m_session_state(ss), m_regexps(r), m_regex_user_states(rs) { }

void FilterProxyModel::force_update ()
{
	//invalidate();
	reset();
}

bool FilterProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & /*sourceParent*/) const
{
	QString file, line;
	int const col_idx = m_session_state.findColumn4Tag(tlv::tag_file);
	if (col_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, col_idx, QModelIndex());
		file = sourceModel()->data(data_idx).toString();
	}
	int const col_idx2 = m_session_state.findColumn4Tag(tlv::tag_line);
	if (col_idx2 >= 0)
	{
		QModelIndex data_idx2 = sourceModel()->index(sourceRow, col_idx2, QModelIndex());
		line = sourceModel()->data(data_idx2).toString();
	}

	bool excluded = false;
	excluded |= m_session_state.isFileLineExcluded(std::make_pair(file.toStdString(), line.toStdString()));

	QString tid;
	int const tid_idx = m_session_state.findColumn4Tag(tlv::tag_tid);
	if (tid_idx >= 0)
	{
		QModelIndex data_idx = sourceModel()->index(sourceRow, tid_idx, QModelIndex());
		tid = sourceModel()->data(data_idx).toString();
	}

	bool regex_accept = true;
	if (m_regexps.size() > 0)
	{
		regex_accept = false;
		QString msg;
		int const msg_idx = m_session_state.findColumn4Tag(tlv::tag_msg);
		if (msg_idx >= 0)
		{
			QModelIndex data_idx = sourceModel()->index(sourceRow, msg_idx, QModelIndex());
			msg = sourceModel()->data(data_idx).toString();
		}

		for (int i = 0, ie = m_regexps.size(); i < ie; ++i)
		{
			if (m_regex_user_states[i])
				regex_accept |= m_regexps[i].exactMatch(msg);
		}
	}

	excluded |= m_session_state.isTIDExcluded(tid.toStdString());

	QModelIndex data_idx = sourceModel()->index(sourceRow, 0, QModelIndex());
	excluded |= m_session_state.isBlockCollapsed(tid, data_idx.row());
	return !excluded && regex_accept;
}

void Connection::onInvalidateFilter ()
{
	if (m_table_view_proxy)
		static_cast<FilterProxyModel *>(m_table_view_proxy)->force_update();
}

void Connection::setFilterFile (int state)
{
	if (state == Qt::Unchecked)
	{
		m_table_view_widget->setModel(m_table_view_proxy->sourceModel());
	}
	else if (state == Qt::Checked)
	{
		if (!m_table_view_proxy)
		{
			m_table_view_proxy = new FilterProxyModel(this, m_main_window->getRegexps(), m_main_window->getRegexUserStates(), m_session_state);

			m_table_view_proxy->setSourceModel(m_table_view_widget->model());
			m_table_view_widget->setModel(m_table_view_proxy);
			m_table_view_proxy->setDynamicSortFilter(true);
		}
		else
		{
			m_table_view_widget->setModel(m_table_view_proxy);
		}
	}
	m_main_window->getTreeViewFile()->setEnabled(m_main_window->filterEnabled());
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

inline QList<QStandardItem *> addRow (QString const & str, bool checked = false)
{
	QList<QStandardItem *> row_items;
	QStandardItem * name_item = new QStandardItem(str);
	name_item->setCheckable(true);
	name_item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
	row_items << name_item;
	return row_items;
}

void Connection::clearFilters (QStandardItem * node)
{
	if (node)
	{
		if (node->checkState() == Qt::Checked)
		{
			node->setCheckState(Qt::Unchecked);
		}
		for (int i = 0, ie = node->rowCount(); i < ie; ++i)
			clearFilters(node->child(i));
	}
}

void Connection::clearFilters ()
{
	QStandardItem * node = m_tree_view_file_model->invisibleRootItem();
	clearFilters(node);
	sessionState().m_file_filters.clear();
	//@TODO: clear TID Filter
}

void Connection::appendToFileFilters (std::string const & item, bool checked)
{
	boost::char_separator<char> sep(":/\\");
	appendToFileFilters(sep, item, checked);
}

void Connection::appendToFileFilters (boost::char_separator<char> const & sep, std::string const & item, bool checked)
{
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
	tokenizer_t tok(item, sep);

	QStandardItem * node = m_tree_view_file_model->invisibleRootItem();
	QStandardItem * last_hidden_node = 0;
	bool append = false;
	bool stop = false;
	for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
	{
		QString qItem = QString::fromStdString(*it);
		QStandardItem * child = findChildByText(node, qItem);
		if (child != 0)
		{
			node = child;
			if (!stop)
			{
				if (child->rowCount() == 1)
				{
					last_hidden_node = node;
				}
				else if (child->rowCount() > 1)
				{
					stop = true;
					last_hidden_node = node;
				}
			}
		}
		else
		{
			stop = true;
			append = true;
			QList<QStandardItem *> row_items = addRow(qItem, checked);
			node->appendRow(row_items);
			node = row_items.at(0);
		}
	}
	if (last_hidden_node)
	{
		m_main_window->getTreeViewFile()->setRootIndex(last_hidden_node->index());
	}
	if (!append && checked)
	{
		node->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
	}
}

void Connection::appendToFileFilters (boost::char_separator<char> const & sep, std::string const & file, std::string const & line)
{
	appendToFileFilters(sep, file + "/" + line);
}

void Connection::appendToTIDFilters (std::string const & item)
{
	QString qItem = QString::fromStdString(item);
	QStandardItem * root = m_list_view_tid_model->invisibleRootItem();
	QStandardItem * child = findChildByText(root, qItem);
	if (child == 0)
	{
		QList<QStandardItem *> row_items = addRow(qItem, false);
		root->appendRow(row_items);
	}
}

bool Connection::appendToFilters (DecodedCommand const & cmd)
{
	std::string line;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_line)
		{
			line = cmd.tvs[i].m_val;
			break;
		}

		if (cmd.tvs[i].m_tag == tlv::tag_tid)
		{
			int idx = sessionState().m_tls.findThreadId(cmd.tvs[i].m_val);
			if (cmd.hdr.cmd == tlv::cmd_scope_entry)
				sessionState().m_tls.incrIndent(idx);
			if (cmd.hdr.cmd == tlv::cmd_scope_exit)
				sessionState().m_tls.decrIndent(idx);
			appendToTIDFilters(cmd.tvs[i].m_val);
		}
	}

	boost::char_separator<char> sep(":/\\");

	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_file)
		{
			std::string file(cmd.tvs[i].m_val);
			appendToFileFilters(sep, file, line);
		}
	}
	return true;
}


//////////////////// storage stuff //////////////////////////////
QString Connection::createStorageName () const
{
	return sessionState().m_name + QString::number(sessionState().m_tab_idx);
}

bool Connection::setupStorage (QString const & name)
{
	if (!m_from_file && !m_storage)
	{
		m_storage = new QFile(name + ".tlv_trace");
		m_storage->open(QIODevice::WriteOnly);
		m_datastream = new QDataStream(m_storage);

		if (!m_datastream)
		{
			return false;
		}
	}
	return true;
}

void Connection::copyStorageTo (QString const & filename)
{
	if (m_storage)
	{
		m_storage->flush();
		m_storage->copy(filename);
	}
	else
	{
		QString name = createStorageName();
		QFile trc(name + ".tlv_trace");
		trc.open(QIODevice::ReadOnly);
		trc.copy(filename);
		trc.close();
	}
}

void Connection::exportStorageToCSV (QString const & filename)
{
	QFile csv(filename);
	csv.open(QIODevice::WriteOnly);
	QTextStream str(&csv);
	for (size_t r = 0, re = m_table_view_widget->model()->rowCount(); r < re; ++r)
	{
		for (size_t c = 0, ce = m_table_view_widget->model()->columnCount(); c < ce; ++c)
		{
			QModelIndex current = m_table_view_widget->model()->index(r, c, QModelIndex());
			QString txt = m_table_view_widget->model()->data(current).toString();
			str << "\"" << txt << "\"";
			if (c < ce - 1)
				str << ",\t";
		}
		str << "\n";
	}
	csv.close();
}

void Connection::closeStorage ()
{
	if (m_storage)
	{
		delete m_datastream;
		m_storage->close();
		delete m_storage;
		m_storage = 0;
	}
}

/*inline void Dump (DecodedCommand const & c)
{
#ifdef _DEBUG
	qDebug("command.hdr.cmd = 0x%x command.hdr.len = 0x%x", c.hdr.cmd, c.hdr.len);
	for (size_t i = 0; i < c.tvs.size(); ++i)
		qDebug("tlv[%u] t=%02x val=%s", i, c.tvs[i].m_tag, c.tvs[i].m_val.c_str());
#endif
}*/


