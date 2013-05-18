#include "connection.h"
#include <QtNetwork>
#include <QMessageBox>
#include <QHeaderView>
#include <tlv_parser/tlv_encoder.h>
#include "logs/logtablemodel.h"
#include "cmd.h"
#include "utils.h"
#include "dock.h"
#include <cstdlib>

inline void Dump (DecodedCommand const & c)
{
	qDebug("command.hdr.cmd = 0x%x command.hdr.len = 0x%x", c.hdr.cmd, c.hdr.len);
	for (size_t i = 0; i < c.tvs.size(); ++i)
		qDebug("tlv[%u] t=%02x val=%s", i, c.tvs[i].m_tag, c.tvs[i].m_val.toStdString().c_str());
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
	LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	size_t const rows = m_decoded_cmds.size();
	model->transactionStart(static_cast<int>(rows));
}

void Connection::onHandleCommands ()
{
	LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
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
	LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());

	setupColumnSizes(false);
	model->transactionCommit();

	if (!m_main_window->filterEnabled() || m_main_window->autoScrollEnabled())
		model->emitLayoutChanged();

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

				qint64 const count = (t->*read_member_fn)(local_buff, static_cast<int>(to_read));
				sessionState().m_recv_bytes += count;			

				if (count <= 0)
				{
					data_in_stream = false;
					break;	// no more data in stream
				}

				for (int i = 0; i < count; ++i)
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
	m_src_stream = e_Stream_TCP;
	m_src_protocol = e_Proto_TLV;

	m_tcpstream = new QTcpSocket(this);
	m_tcpstream->setSocketDescriptor(sd);
	connect(this, SIGNAL(handleCommands()), this, SLOT(onHandleCommands()));
}

void Connection::setImportFile (QString const & fname)
{
	m_src_stream = e_Stream_File;
	m_src_protocol = e_Proto_TLV;
}

void Connection::setTailFile (QString const & fname)
{
	m_src_stream = e_Stream_File;
	m_src_protocol = e_Proto_CSV;

	QFile * f = new QFile(fname);
	if (!f->open(QIODevice::ReadOnly))
	{
		QMessageBox::critical(0, tr("Error"), tr("Could not open file"));
		delete f;
		return;
	}

	m_file_csv_stream = new QTextStream(f);
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
	connect(this, SIGNAL(handleCommands()), this, SLOT(onHandleCommands()));

	int const res = processStream(&stream, &QDataStream::readRawData);
	switch (res)
	{
		case e_data_ok:
		{
			// update column sizes
			columns_sizes_t const & sizes = *sessionState().m_columns_sizes;
			bool const old = m_table_view_widget->blockSignals(true);
			for (int c = 0, ce = sizes.size(); c < ce; ++c)
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

void Connection::processTailCSVStream ()
{
	while (!m_file_csv_stream->atEnd())
	{
		while (!m_decoded_cmds.full() && !m_file_csv_stream->atEnd())
		{
			QString const data = m_file_csv_stream->readLine(2048);
			tlv::TV tv;
			tv.m_tag = tlv::tag_msg;
			tv.m_val = data;
			m_current_cmd.tvs.push_back(tv);
			m_decoded_cmds.push_back(m_current_cmd);
			m_current_cmd.Reset(); // reset current command for another decoding pass
		}

		if (m_decoded_cmds.size() > 0)
		{
			emit onHandleCommandsStart();
			LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
			for (size_t i = 0, ie = m_decoded_cmds.size(); i < ie; ++i)
			{
				DecodedCommand & cmd = m_decoded_cmds.front();
				handleCSVStreamCommand(cmd);
				m_decoded_cmds.pop_front();
			}

			emit onHandleCommandsCommit();
		}
	}

	QTimer::singleShot(250, this, SLOT(processTailCSVStream()));
}

void item2separator (QString const & item, QString & sep)
{
	if (item == "\\t")
		sep = "\t";
	else if (item == "\\n")
		sep = "\n";
	else
		sep = item;
}

bool Connection::handleCSVStreamCommand (DecodedCommand const & cmd)
{
	if (!m_session_state.isConfigured())
	{
		if (m_session_state.separatorChar().isEmpty())
		{
			m_main_window->onSetupCSVSeparator(m_session_state.getAppIdx(), true);
			item2separator(m_main_window->separatorChar(), m_session_state.m_csv_separator);
		}

		QString const & val = cmd.tvs[0].m_val;
		QStringList const list = val.split(m_session_state.separatorChar());
		int const cols = list.size();

		int const idx = m_session_state.m_app_idx;
		if (m_main_window->m_config.m_columns_setup[idx].size() == 0)
		{
			m_main_window->m_config.m_columns_setup[idx].reserve(cols);
			m_main_window->m_config.m_columns_sizes[idx].reserve(cols);
			m_main_window->m_config.m_columns_align[idx].reserve(cols);
			m_main_window->m_config.m_columns_elide[idx].reserve(cols);

			if (idx >= 0 && idx < m_main_window->m_config.m_columns_setup.size())
				for (int i = 0; i < cols; ++i)
				{
					m_main_window->m_config.m_columns_setup[idx].push_back(tr("Col_%1").arg(i));
					m_main_window->m_config.m_columns_sizes[idx].push_back(127);
					m_main_window->m_config.m_columns_align[idx].push_back(QString(alignToString(e_AlignLeft)));
					m_main_window->m_config.m_columns_elide[idx].push_back(QString(elideToString(e_ElideRight)));
				}
		}

		m_main_window->onSetupCSVColumns(m_session_state.getAppIdx(), cols, true);
	}

	//appendToFilters(cmd);
	LogTableModel * model = static_cast<LogTableModel *>(m_table_view_proxy ? m_table_view_proxy->sourceModel() : m_table_view_widget->model());
	model->appendCommandCSV(m_table_view_proxy, cmd);
	return true;
}

bool Connection::handlePingCommand (DecodedCommand const & cmd)
{
	qDebug("ping from client!");
	QWidget * w = m_tab_widget;
	if (w)
	{
		disconnect(m_tcpstream, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
		m_marked_for_close = true;
		QTimer::singleShot(0, static_cast<Server *>(parent()), SLOT(onCloseMarkedTabs()));
	}
	return true;
}

bool Connection::handleShutdownCommand (DecodedCommand const & cmd)
{
	qDebug("shutdown from client requested (update?)");
	m_main_window->onQuit();
	return true;
}

bool Connection::handleDictionnaryCtx (DecodedCommand const & cmd)
{
	qDebug("received custom context dictionnary");
	QList<QString> name;
	QList<QString> value;
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; i+=2)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_string)
		{
			name.append(cmd.tvs[i].m_val);
		}

		if (cmd.tvs[i+1].m_tag == tlv::tag_int)
		{
			value.append(cmd.tvs[i+1].m_val);
		}
	}
	sessionState().addCtxDict(name, value);
	return true;
}

bool Connection::tryHandleCommand (DecodedCommand const & cmd)
{
	switch (cmd.hdr.cmd)
	{
		case tlv::cmd_setup:			handleSetupCommand(cmd); break;
		case tlv::cmd_log:				handleLogCommand(cmd); break;
		case tlv::cmd_log_clear:		handleLogClearCommand(cmd); break;
		case tlv::cmd_plot_xy:			handleDataXYCommand(cmd); break;
		case tlv::cmd_plot_xyz:			handleDataXYZCommand(cmd); break;
		case tlv::cmd_table_xy:			handleTableXYCommand(cmd); break;
		case tlv::cmd_table_setup:		handleTableSetupCommand(cmd); break;
		case tlv::cmd_scope_entry:		handleLogCommand(cmd); break;
		case tlv::cmd_scope_exit:		handleLogCommand(cmd); break;
		case tlv::cmd_save_tlv:			handleSaveTLVCommand(cmd); break;
		case tlv::cmd_export_csv:		handleExportCSVCommand(cmd); break;
		case tlv::cmd_ping:				handlePingCommand(cmd); break;
		case tlv::cmd_shutdown:			handleShutdownCommand(cmd); break;
		case tlv::cmd_dict_ctx:			handleDictionnaryCtx(cmd); break;
		case tlv::cmd_gantt_bgn:		handleGanttBgnCommand(cmd); break;
		case tlv::cmd_gantt_end:		handleGanttEndCommand(cmd); break;
		case tlv::cmd_gantt_frame_bgn:	handleGanttFrameBgnCommand(cmd); break;
		case tlv::cmd_gantt_frame_end:	handleGanttFrameEndCommand(cmd); break;
		case tlv::cmd_plot_clear:		handlePlotClearCommand(cmd); break;
		case tlv::cmd_table_clear:		handleTableClearCommand(cmd); break;
		case tlv::cmd_gantt_clear:		handleGanttClearCommand(cmd); break;

		default: qDebug("unknown command, ignoring\n"); break;
	}

	if (m_src_stream == e_Stream_TCP && m_tcp_dump_stream)
		m_tcp_dump_stream->writeRawData(&cmd.orig_message[0], cmd.hdr.len + tlv::Header::e_Size);
	return true;
}


//////////////////// storage stuff //////////////////////////////
QString Connection::createStorageName () const
{
	return sessionState().m_app_name + QString::number(sessionState().m_storage_idx);
}

bool Connection::setupStorage (QString const & name)
{
	if (m_src_stream == e_Stream_TCP && !m_storage)
	{
		m_storage = new QFile(name + ".tlv_trace");
		m_storage->open(QIODevice::WriteOnly);
		m_tcp_dump_stream = new QDataStream(m_storage);

		if (!m_tcp_dump_stream)
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

bool Connection::handleSaveTLVCommand (DecodedCommand const & cmd)
{
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_file)
		{
			copyStorageTo(cmd.tvs[i].m_val);
			return true;
		}
	}
	return true;
}

void Connection::exportStorageToCSV (QString const & filename)
{
	// " --> ""
	QRegExp regex("\"");
	QString to_string("\"\"");
	QFile csv(filename);
	csv.open(QIODevice::WriteOnly);
	QTextStream str(&csv);

	for (int c = 0, ce = m_session_state.m_columns_setup_current->size(); c < ce; ++c)
	{
		str << "\"" << m_session_state.m_columns_setup_current->at(c) << "\"";
		if (c < ce - 1)
			str << ",\t";
	}
	str << "\n";

	for (int r = 0, re = m_table_view_widget->model()->rowCount(); r < re; ++r)
	{
		for (int c = 0, ce = m_table_view_widget->model()->columnCount(); c < ce; ++c)
		{
			QModelIndex current = m_table_view_widget->model()->index(r, c, QModelIndex());
			// csv nedumpovat pres proxy
			QString txt = m_table_view_widget->model()->data(current).toString();
			QString const quoted_txt = txt.replace(regex, to_string);
			str << "\"" << quoted_txt << "\"";
			if (c < ce - 1)
				str << ",\t";
		}
		str << "\n";
	}
	csv.close();
}

bool Connection::handleExportCSVCommand (DecodedCommand const & cmd)
{
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_file)
		{
			exportStorageToCSV(cmd.tvs[i].m_val);
			return true;
		}
	}
	return false;
}

void Connection::closeStorage ()
{
	if (m_storage)
	{
		delete m_tcp_dump_stream;
		m_storage->close();
		m_storage->remove();
		delete m_storage;
		m_storage = 0;
	}
}


