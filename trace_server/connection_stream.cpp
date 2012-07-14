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
		if (m_main_window->scopesEnabled())
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

void Connection::onHandleCommandsStart ()
{
	ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	size_t const rows = m_decoded_cmds.size();
	model->transactionStart(rows);
}

void Connection::onHandleCommands ()
{
	ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	size_t const rows = m_decoded_cmds.size();
	for (size_t i = 0; i < rows; ++i)
	{
		DecodedCommand & cmd = m_decoded_cmds.front();
		tryHandleCommand(cmd);
		m_decoded_cmds.pop_front();
	}
}

void Connection::onHandleCommandsCommit ()
{
	ModelView * model = static_cast<ModelView *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());

	setupColumnSizes();
	model->transactionCommit();

	if (m_main_window->autoScrollEnabled())
		m_table_view_widget->scrollToBottom();
}


template <class T, typename T_Ret, typename T_Arg0, typename T_Arg1>
int Connection::processStream (T * t, T_Ret (T::*read_member_fn)(T_Arg0, T_Arg1))
{
	enum { local_buff_sz = 128 };
	char local_buff[local_buff_sz];
	try {

		emit onHandleCommandsStart();
		bool data_in_stream = true;
		while (data_in_stream)
		{
			// read data into ring buffer
			while (!m_buffer.full())
			{
				size_t const free_space = m_buffer.reserve();
				size_t const to_read = free_space < local_buff_sz ? free_space : local_buff_sz;

				qint64 const count = (t->*read_member_fn)(local_buff, to_read);
				sessionState().m_recv_bytes += count;			

				if (count <= 0)
				{
					data_in_stream = false;
					break;	// no more data in stream
				}

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
		
		emit onHandleCommandsCommit();
		return e_data_ok;
	}
	catch (std::out_of_range const & e)
	{
		QMessageBox::critical(0, tr("My Application"),
								tr("OOR exception during decoding: %1").arg(e.what()),
								QMessageBox::Ok, QMessageBox::Ok);	
		return e_data_decode_oor;
	}
	catch (std::length_error const & e)
	{
		QMessageBox::critical(0, tr("My Application"),
								tr("LE exception during decoding: %1").arg(e.what()),
								QMessageBox::Ok, QMessageBox::Ok);
		return e_data_decode_lnerr;
	}
	catch (std::exception const & e)
	{
		QMessageBox::critical(0, tr("My Application"),
								tr("generic exception during decoding: %1").arg(e.what()),
								QMessageBox::Ok, QMessageBox::Ok);
		return e_data_decode_captain_failure;
	}
	catch (...)
	{
		QMessageBox::critical(0, tr("My Application"),
								tr("... exception during decoding"),
								QMessageBox::Ok, QMessageBox::Ok);
		return e_data_decode_general_failure;
	}
	return e_data_decode_error;
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

	int const res = processStream(&stream, &QDataStream::readRawData);
	switch (res)
	{
		case e_data_ok:
		{
			// update column sizes
			columns_sizes_t const & sizes = *sessionState().m_columns_sizes;
			bool const old = m_table_view_widget->blockSignals(true);
			for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
				m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
			m_table_view_widget->blockSignals(old);
			return;
		}
		case e_data_decode_oor:
		case e_data_decode_lnerr:
		case e_data_decode_captain_failure:
		case e_data_decode_general_failure:
		case e_data_decode_error:
			qDebug("!!! exception duging decoding!");
	}
}

bool Connection::tryHandleCommand (DecodedCommand const & cmd)
{
	switch (cmd.hdr.cmd)
	{
		case tlv::cmd_setup:		handleSetupCommand(cmd); break;
		case tlv::cmd_log:			handleLogCommand(cmd); break;
		case tlv::cmd_scope_entry:	handleLogCommand(cmd); break;
		case tlv::cmd_scope_exit:	handleLogCommand(cmd); break;
		//case tlv::cmd_save_as:		handleSaveAsCommand(cmd); break;
		default: qDebug("unknown command, ignoring\n"); break;
	}

	if (!m_from_file && m_datastream)
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


