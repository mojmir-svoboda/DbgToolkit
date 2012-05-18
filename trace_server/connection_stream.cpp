#include "connection.h"
#include <QtNetwork>
#include <QMessageBox>
#include <QHeaderView>
#include <tlv_parser/tlv_encoder.h>
#include "modelview.h"
#include "cmd.h"

/*inline void Dump (DecodedCommand const & c)
{
#ifdef _DEBUG
	qDebug("command.hdr.cmd = 0x%x command.hdr.len = 0x%x", c.hdr.cmd, c.hdr.len);
	for (size_t i = 0; i < c.tvs.size(); ++i)
		qDebug("tlv[%u] t=%02x val=%s", i, c.tvs[i].m_tag, c.tvs[i].m_val.c_str());
#endif
}*/

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
		columns_sizes_t const & sizes = m_main_window->getColumnSizes(sessionState().m_app_idx);
		columns_setup_t const & global_template = m_main_window->getColumnSetup(sessionState().m_app_idx);

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

//////////////// PERF!!!!! //////////////////
	/*{
		// hotfix for disobedient column hiding @TODO: resolve in future
		columns_sizes_t const & sizes = m_main_window->getColumnSizes(sessionState().m_app_idx);
		columns_setup_t const & global_template = m_main_window->getColumnSetup(sessionState().m_app_idx);
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
	}*/

//////////////// PERF!!!!! //////////////////

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

	// update column sizes
	columns_sizes_t const & sizes = *sessionState().m_columns_sizes;
	bool const old = m_table_view_widget->blockSignals(true);
	for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
		m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
	m_table_view_widget->blockSignals(old);
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
		m_storage->remove();
		delete m_storage;
		m_storage = 0;
	}
}


