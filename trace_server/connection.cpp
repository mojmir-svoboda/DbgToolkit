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
#include "modelview.h"
#include <boost/tokenizer.hpp>
#include "../tlv_parser/tlv_encoder.h"
#include "../trace_client/trace.h"

Connection::Connection (QObject * parent)
	: QThread(parent)
	, m_main_window(0)
	, m_from_file(false)
	, m_first_line(true)
	, m_table_view_widget(0)
	, m_tree_view_file_model(0)
	, m_tree_view_func_model(0)
	, m_table_view_proxy(0)
	, m_buffer(e_ringbuff_size)
	, m_current_cmd()
	, m_decoded_cmds(e_ringcmd_size)
	, m_decoder()
	, m_storage(0)
	, m_datastream(0)
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
}

void Connection::onLevelValueChanged (int val)
{
	char tlv_buff[16];
	int const result = _snprintf(tlv_buff, 16, "%u", val);

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

	if (m_first_line)	// resize columns according to saved template
	{
		MainWindow::columns_sizes_t const & sizes = m_main_window->getColumnSizes(sessionState().m_app_idx);
		for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
		{
			m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
		}
		m_first_line = false;
	}

	model->transactionCommit();

	if (m_main_window->autoScollEnabled())
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
			sessionState().m_name = app_name;
			m_main_window->getTabTrace()->setTabText(sessionState().m_tab_idx, app_name);
			QString storage_name = createStorageName();
			setupStorage(storage_name);

			sessionState().m_app_idx = m_main_window->findAppName(app_name);
			sessionState().m_columns_setup = &m_main_window->getColumnSetup(sessionState().m_app_idx);
			if (m_main_window->getColumnSetup(sessionState().m_app_idx).size())	// load if config already exists
			{
				sessionState().setupColumns(&m_main_window->getColumnSetup(sessionState().m_app_idx), &m_main_window->getColumnSizes(sessionState().m_app_idx));
			}

			m_current_cmd.tvs.reserve(sessionState().m_columns_setup->size());

			for (size_t i = 0, ie = sessionState().m_columns_setup->size(); i < ie; ++i)
			{
				m_table_view_widget->model()->insertColumn(i);
			}

			sessionState().m_columns_sizes = &m_main_window->getColumnSizes(sessionState().m_app_idx);
			MainWindow::columns_sizes_t const & sizes = *sessionState().m_columns_sizes;
			for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
			{
				m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
			}
			static_cast<ModelView *>(m_table_view_widget->model())->emitLayoutChanged();
		}
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
FilterProxyModel::FilterProxyModel (QObject * parent, SessionState & ss) : QSortFilterProxyModel(parent), m_session_state(ss) { }

void FilterProxyModel::force_update()
{
	invalidate();
}

bool FilterProxyModel::filterAcceptsRow (int sourceRow, QModelIndex const & sourceParent) const
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

	bool const excluded = m_session_state.isFileLineExcluded(std::make_pair(file.toStdString(), line.toStdString()));
	return !excluded;
}

void Connection::onFilterFile (int state)
{
	if (state == Qt::Unchecked)
	{
		m_table_view_widget->setModel(m_table_view_proxy->sourceModel());
	}
	else if (state == Qt::Checked)
	{
		if (!m_table_view_proxy)
		{
			m_table_view_proxy = new FilterProxyModel(this, m_session_state);

			m_table_view_proxy->setSourceModel(m_table_view_widget->model());
			m_table_view_widget->setModel(m_table_view_proxy);
			m_table_view_proxy->setDynamicSortFilter(true);

			connect(m_table_view_proxy->sourceModel(), SIGNAL(dataChanged(QModelIndex, QModelIndex)), m_table_view_proxy, SLOT(dataChanged(QModelIndex, QModelIndex)));
		}
		else
		{
			m_table_view_widget->setModel(m_table_view_proxy);
		}
	}
}

void Connection::setupModelFile ()
{
	m_tree_view_file_model = new QStandardItemModel;
	m_main_window->getTreeViewFile()->setModel(m_tree_view_file_model);
	//main_window->getTreeViewFile()->expandAll();
}

void Connection::onApplyFilterClicked ()
{
	if (m_table_view_proxy)
		static_cast<FilterProxyModel *>(m_table_view_proxy)->force_update();
}

inline QList<QStandardItem *> addRow (QString const & str)
{
	QList<QStandardItem *> row_items;
	QStandardItem * name_item = new QStandardItem(str);
	row_items << name_item;
	return row_items;
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
		}
	}

	boost::char_separator<char> sep(":/\\");

	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_file)
		{
			typedef boost::tokenizer<boost::char_separator<char> > tokenizer_t;
			std::string file(cmd.tvs[i].m_val);
			tokenizer_t tok(file, sep);

			QStandardItem * item = m_tree_view_file_model->invisibleRootItem();
			for (tokenizer_t::const_iterator it = tok.begin(), ite = tok.end(); it != ite; ++it)
			{
				QString qfile = QString::fromStdString(*it);
				QStandardItem * child = findChildByText(item, qfile);
				if (child != 0)
					item = child;
				else
				{
					QList<QStandardItem *> row_items = addRow(qfile);
					item->appendRow(row_items);
					item = row_items.at(0);
				}
			}

			if (!line.empty())
			{
				QString qline = QString::fromStdString(line);
				QStandardItem * last_level = findChildByText(item, qline);
				if (last_level == 0)
				{
					QList<QStandardItem *> row_items = addRow(qline);
					item->appendRow(row_items);
					item = row_items.at(0);
				}
			}
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


