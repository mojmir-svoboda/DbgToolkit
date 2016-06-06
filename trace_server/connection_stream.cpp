#include "connection.h"
#include <QtNetwork>
#include <QMessageBox>
#include <QHeaderView>
#include <trace_proto/trace_proto.h>
//#include "utils.h"
#include <utils/utils_boost.h>
//#include "dock.h"
#include "mainwindow.h"
#include <cstdlib>

// bool isCommandQueuable (tlv::tag_t cmd)
// {
// 	switch (cmd)
// 	{
// 		case tlv::cmd_setup:			return false;
// 		case tlv::cmd_ping:				return false;
// 		case tlv::cmd_shutdown:			return false;
// 		case tlv::cmd_dict_ctx:			return false;
// 		case tlv::cmd_save_tlv:			return false;
// 		case tlv::cmd_export_csv:		return false;
// 		case tlv::cmd_plot_clear:		return false;
// 		case tlv::cmd_table_setup:		return false;
// 		case tlv::cmd_table_clear:		return false;
// 		case tlv::cmd_log_clear:		return false;
// 		case tlv::cmd_log:
// 		case tlv::cmd_scope_entry:
// 		case tlv::cmd_scope_exit:
// 		case tlv::cmd_plot_xy:
// 		case tlv::cmd_plot_xyz:	
// 		case tlv::cmd_table_xy:	
// 		case tlv::cmd_gantt_bgn:
// 		case tlv::cmd_gantt_end:
// 		case tlv::cmd_gantt_frame_bgn:
// 		case tlv::cmd_gantt_frame_end:
// 		case tlv::cmd_gantt_clear:		return true;
// 
// 		default:
// 			qWarning("unknown command!\n"); 
// 			return true;
// 	}
// }

// E_DataWidgetType queueForCommand (tlv::tag_t cmd)
// {
// 	switch (cmd)
// 	{
// 		case tlv::cmd_save_tlv:			return e_data_log;
// 		case tlv::cmd_export_csv:		return e_data_log; 
// 		case tlv::cmd_log:				return e_data_log;
// 		case tlv::cmd_log_clear:		return e_data_log;
// 		case tlv::cmd_scope_entry:		return e_data_log;
// 		case tlv::cmd_scope_exit:		return e_data_log;
// 		case tlv::cmd_plot_xy:			return e_data_plot;
// 		case tlv::cmd_plot_xyz:			return e_data_plot;
// 		case tlv::cmd_plot_clear:		return e_data_plot;
// 		case tlv::cmd_table_xy:			return e_data_table;
// 		case tlv::cmd_table_setup:		return e_data_table;
// 		case tlv::cmd_table_clear:		return e_data_table;
// 		case tlv::cmd_gantt_bgn:		return e_data_gantt;
// 		case tlv::cmd_gantt_end:		return e_data_gantt;
// 		case tlv::cmd_gantt_frame_bgn:	return e_data_gantt;
// 		case tlv::cmd_gantt_frame_end:	return e_data_gantt;
// 		case tlv::cmd_gantt_clear:		return e_data_gantt;
// 
// 		default: qWarning("unknown command!\n"); return e_data_widget_max_value;
// 	}
// }

namespace {
	struct EnqueueCommand {
		E_DataWidgetType m_type;
		DecodedCommand const & m_cmd;

		EnqueueCommand (E_DataWidgetType t, DecodedCommand const & cmd) : m_type(t), m_cmd(cmd) { }
		
		template <typename T>
		void operator() (T & t)
		{
			if (t.e_type == m_type)
				t.m_queue.push_back(m_cmd);
		}
	};
}

bool Connection::enqueueCommand (DecodedCommand const & cmd)
{
// 	E_DataWidgetType const queue_type = queueForCommand(cmd.m_hdr.cmd);
// 	if (queue_type == e_data_widget_max_value)
// 		return false;
// 
// 	recurse(m_data_widgets, EnqueueCommand(queue_type, cmd));
	return true;
}


bool Connection::tryHandleCommand (DecodedCommand const & cmd, E_ReceiveMode mode)
{
	switch (cmd.present)
	{
		case Command_PR_config:			handleConfigCommand(cmd); break;
//		case tlv::cmd_shutdown:			handleShutdownCommand(cmd); break;
// 		case tlv::cmd_ping:				handlePingCommand(cmd); break;
// 		case tlv::cmd_save_tlv:			handleSaveTLVCommand(cmd); break;
// 		case tlv::cmd_export_csv:		handleExportCSVCommand(cmd); break;
		case Command_PR_dict:				handleDictionary(cmd); break;

		case Command_PR_log:				handleLogCommand(cmd, mode); break;
// 		case tlv::cmd_log_clear:		handleLogClearCommand(cmd, mode); break;
 		case Command_PR_plot:				handlePlotCommand(cmd, mode); break;
		case Command_PR_plotm:			handlePlotCommand(cmd, mode); break;
		case Command_PR_snd:				handleSoundCommand(cmd, mode); break;
// 		case tlv::cmd_plot_xyz:			handlePlotCommand(cmd, mode); break;
// 		case tlv::cmd_plot_clear:		handlePlotCommand(cmd, mode); break;
// 		case tlv::cmd_table_xy:			handleTableCommand(cmd, mode); break;
// 		case tlv::cmd_table_setup:		handleTableCommand(cmd, mode); break;
// 		case tlv::cmd_table_clear:		handleTableCommand(cmd, mode); break;
// 		case tlv::cmd_gantt_bgn:		handleGanttBgnCommand(cmd, mode); break;
// 		case tlv::cmd_gantt_end:		handleGanttEndCommand(cmd, mode); break;
// 		case tlv::cmd_gantt_frame_bgn:	handleGanttFrameBgnCommand(cmd, mode); break;
// 		case tlv::cmd_gantt_frame_end:	handleGanttFrameEndCommand(cmd, mode); break;
// 		case tlv::cmd_gantt_clear:		handleGanttClearCommand(cmd, mode); break;

		default: qDebug("unknown command, ignoring\n"); break;
	}
	return true;
}

void Connection::onHandleCommands ()
{
// 	size_t const rows = m_decoded_cmds.size();
// 	for (size_t i = 0; i < rows; ++i)
// 	{
// 		DecodedCommand & cmd = m_decoded_cmds.front();
// 		if (isCommandQueuable(cmd.m_hdr.cmd))
// 		{
// 			enqueueCommand(cmd);
// 		}
// 		else
// 		{
// 			tryHandleCommand(cmd, e_RecvSync);
// 		}
// 
// 		if (m_src_stream == e_Stream_TCP && m_tcp_dump_stream)
// 			m_tcp_dump_stream->writeRawData(&cmd.m_orig_message[0], cmd.m_hdr.len + tlv::Header::e_Size);
// 
// 		m_decoded_cmds.pop_front();
// 	}
}

void Connection::onHandleCommandsStart ()
{
}

namespace {
	struct DequeueCommand {
		Connection & m_conn;

		DequeueCommand ( Connection & c) : m_conn(c) { }
		
		template <typename T>
		void operator() (T & t)
		{
			foreach (typename T::widget_t * w, t)
				w->commitCommands(e_RecvBatched);
		}
	};
}

void Connection::onHandleCommandsCommit ()
{
	recurse(m_data_widgets, DequeueCommand(*this));
}


template <class T, typename T_Ret, typename T_Arg0, typename T_Arg1>
int Connection::processStream (T * t, T_Ret (T::*read_member_fn)(T_Arg0, T_Arg1))
{
	try {
		emit onHandleCommandsStart(); // @TODO unused, remove
		size_t const hdr_sz = sizeof(asn1::Header);
		size_t batch_size = 0;

		while (1)
		{
			if (!m_dcd_ctx.m_has_hdr)
			{
				char * const buff_ptr = m_dcd_ctx.getEndPtr();
				size_t const curr_sz = m_dcd_ctx.getSize();
				size_t const to_recv = hdr_sz - curr_sz;
				qint64 const count = (t->*read_member_fn)(buff_ptr, to_recv); // read header
				if (count <= 0)
				{
					break;	 // not enough data (can use break, no data was read)
				}
				else
				{
					m_dcd_ctx.moveEndPtr(count);
					Stats::get().m_received_bytes += count;

					if (m_dcd_ctx.getSize() == hdr_sz)
					{
						m_dcd_ctx.m_has_hdr = true; // message header ready
					}
					else
					{
						// @TODO: maybe use same algo as in (**) i.e. flush and return
						return e_data_need_more; // not enough data
					}
				}
			}
			else
			{
				if (!m_dcd_ctx.m_has_payload)
				{
					char * const buff_ptr = m_dcd_ctx.getEndPtr();
					asn1::Header const & hdr = m_dcd_ctx.getHeader();
					size_t const curr_sz = m_dcd_ctx.getSize() - hdr_sz;
					size_t const to_recv = hdr.m_len - curr_sz;
					qint64 const count = (t->*read_member_fn)(buff_ptr, to_recv); // read payload
					if (count <= 0)
					{
						// @TODO: maybe use same algo as in (**) i.e. flush and return
						return e_data_need_more; // not enough data
					}
					else
					{
						m_dcd_ctx.moveEndPtr(count);
						Stats::get().m_received_bytes += count;

						if (m_dcd_ctx.getSize() == hdr_sz + hdr.m_len)
						{
							m_dcd_ctx.m_has_payload = true; // message payload ready
						}
						else
						{
							// @TODO: maybe use same algo as in (**) i.e. flush and return
							return e_data_need_more; // not enough data
						}
					}
				}

				if (m_dcd_ctx.m_has_payload)
				{
					asn1::Header const & hdr = m_dcd_ctx.getHeader();
					assert(hdr.m_version == 1);
					if (hdr.m_version == 1)
					{
						Command * cmd_ptr = &m_dcd_ctx.m_command;
						void * cmd_void_ptr = cmd_ptr;
						char const * payload = m_dcd_ctx.getPayload();
						asn1::Header const & hdr = m_dcd_ctx.getHeader();
						size_t const av = m_asn1_allocator.available();
						size_t const size_estimate = hdr.m_len * 4; // at most
						sys::hptimer_t const now = sys::queryTime_us();
						m_dcd_ctx.m_command.m_stime = now;
						if (av < size_estimate)
						{	
							// not enough memory for asn1 decoder (**)
							Stats::get().m_decoder_mem_asn1_realloc_count++;
							// 1) flush everything
							emit onHandleCommandsCommit();
							batch_size = 0;
							Stats::get().m_received_batches++;
							// 2) resize 
							m_asn1_allocator.Reset();
							//m_dcd_ctx.resetCurrentCommand();
							m_asn1_allocator.resizeStorage(m_asn1_allocator.calcNextSize());
							Stats::get().updateDecoderMemAsn1Max(m_asn1_allocator.capacity());
							// 3) check
							size_t const av = m_asn1_allocator.available();
							if (av < size_estimate)
							{
								Q_ASSERT(0);
							}
						}

						const asn_dec_rval_t rval = ber_decode(&m_asn1_allocator, 0, &asn_DEF_Command, &cmd_void_ptr, payload, hdr.m_len);
						if (rval.code != RC_OK)
						{
							QMessageBox::critical(0, tr("trace server"), tr("Decoder exception: Error while decoding ASN1: err=%1, consumed %2 bytes").arg(rval.code).arg(rval.consumed), QMessageBox::Ok, QMessageBox::Ok);

							m_dcd_ctx.resetCurrentCommand();
							m_asn1_allocator.Reset();
							Stats::get().m_received_failed_cmds++;
							return e_data_decode_oor;
						}
						else
							Stats::get().m_received_cmds++;

						tryHandleCommand(m_dcd_ctx.m_command, e_RecvBatched);
						batch_size++;

						Stats::get().updateDecoderMemRecvBuffMax(m_dcd_ctx.capacity());
						m_dcd_ctx.resetCurrentCommand(); // reset current decoder command for another decoding pass

						if (batch_size >= 128)
						{
							break;
						}
					}
				}
			}
		}

		if (batch_size > 0)
		{		
			emit onHandleCommandsCommit();

			m_dcd_ctx.resetCurrentCommand();
			Stats::get().updateDecoderMemAsn1Max(m_asn1_allocator.capacity());
			Stats::get().m_received_batches++;
			m_asn1_allocator.Reset();
		}

		return e_data_ok;
	}
	catch (std::out_of_range const & e)
	{
		QMessageBox::critical(0, tr("trace server"),
								tr("OOR exception during decoding: %1").arg(e.what()),
								QMessageBox::Ok, QMessageBox::Ok);	
		return e_data_decode_oor;
	}
	catch (std::length_error const & e)
	{
		QMessageBox::critical(0, tr("trace server"),
								tr("LE exception during decoding: %1").arg(e.what()),
								QMessageBox::Ok, QMessageBox::Ok);
		return e_data_decode_lnerr;
	}
	catch (std::exception const & e)
	{
		QMessageBox::critical(0, tr("trace server"),
								tr("generic exception during decoding: %1").arg(e.what()),
								QMessageBox::Ok, QMessageBox::Ok);
		return e_data_decode_captain_failure;
	}
	catch (...)
	{
		QMessageBox::critical(0, tr("trace server"),
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
	m_src_protocol = e_Proto_ASN1;
	m_src_name = QString("tcp ").arg(sd);

	m_tcpstream = new QTcpSocket(this);
	m_tcpstream->setSocketDescriptor(sd);
	connect(this, SIGNAL(handleCommands()), this, SLOT(onHandleCommands()));
}

void Connection::setImportFile (QString const & fname)
{
	m_src_stream = e_Stream_File;
	m_src_protocol = e_Proto_TraceFile;
	m_src_name = fname;
}

void Connection::setTailFile (QString const & fname)
{
	m_src_stream = e_Stream_File;
	m_src_protocol = e_Proto_CSV;
	m_src_name = fname;

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
		while (m_tcpstream->bytesAvailable() < sizeof(asn1::Header))
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
			/*columns_sizes_t const & sizes = *sessionState().m_columns_sizes;
			bool const old = m_table_view_widget->blockSignals(true);
			for (int c = 0, ce = sizes.size(); c < ce; ++c)
				m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
			m_table_view_widget->blockSignals(old);*/
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

#ifdef WIN32
#	include <io.h>
# include "platform.h"
	inline qint64 getFileSize (QFile * f)
	{
		qint64 sz = -1;
		if (f)
		{
			LARGE_INTEGER arg;
			HANDLE h = (HANDLE)_get_osfhandle(f->handle());
			if (::GetFileSizeEx(h, &arg) != 0)
				sz = arg.QuadPart;
		}
		return sz;
	}
#endif

void Connection::processTailCSVStream ()
{
// 	qint64 const sz = getFileSize(qobject_cast<QFile *>(m_file_csv_stream->device()));
// 	if (sz > m_file_size)
// 		m_file_size = sz;
// 	
// 	if (sz < m_file_size)
// 	{
// 		m_file_size = 0;
// 		m_main_window->requestReloadFile(m_src_name);
// 		QTimer::singleShot(32, m_main_window, SLOT(onReloadFile()));
// 		m_main_window->onCloseConnection(this); // deletes it immeadiately
// 		return;
// 	}
// 	while (!m_file_csv_stream->atEnd())
// 	{
// 		while (!m_decoded_cmds.full() && !m_file_csv_stream->atEnd())
// 		{
// 			QString const data = m_file_csv_stream->readLine(2048);
// 			QByteArray ba = data.toLatin1();
// 			memcpy(&m_current_cmd.m_orig_message[0], ba.data(), ba.size() < 2048? ba.size() : 2048 - 1);
// 
// 			m_decoded_cmds.push_back(m_current_cmd);
// 			m_current_cmd.reset(); // reset current command for another decoding pass
// 		}
// 
// 		if (m_decoded_cmds.size() > 0)
// 		{
// 			emit onHandleCommandsStart();
// 
// 			for (size_t i = 0, ie = m_decoded_cmds.size(); i < ie; ++i)
// 			{
// 				DecodedCommand & cmd = m_decoded_cmds.front();
// 				handleCSVStreamCommand(cmd);
// 				m_decoded_cmds.pop_front();
// 			}
// 
// 			emit onHandleCommandsCommit();
// 		}
// 	}

	QTimer::singleShot(250, this, SLOT(processTailCSVStream()));
}

bool Connection::handlePingCommand (DecodedCommand const & cmd)
{
	qDebug("ping from client!");
	disconnect(m_tcpstream, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	m_marked_for_close = true;
	QTimer::singleShot(0, m_main_window, SLOT(onCloseMarkedTabs()));
	return true;
}

bool Connection::handleShutdownCommand (DecodedCommand const & cmd)
{
	qDebug("shutdown from client requested (update?)");
	m_main_window->onQuit();
	return true;
}

//////////////////// storage stuff //////////////////////////////
QString Connection::createStorageName () const
{
	return m_app_name + QString::number(m_storage_idx);
}

bool Connection::setupStorage (QString const & name)
{
	if (m_src_stream == e_Stream_TCP && !m_storage)
	{
		m_storage = new QFile(name + "." + g_traceFileExtTLV);
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
		QFile trc(name + "." + g_traceFileExtTLV);
		trc.open(QIODevice::ReadOnly);
		trc.copy(filename);
		trc.close();
	}
}

// bool Connection::handleSaveTLVCommand (DecodedCommand const & cmd)
// {
// 	for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i)
// 	{
// 		if (cmd.m_tvs[i].m_tag == tlv::tag_file)
// 		{
// 			copyStorageTo(cmd.m_tvs[i].m_val);
// 			return true;
// 		}
// 	}
// 	return true;
// }

// bool Connection::handleExportCSVCommand (DecodedCommand const & cmd)
// {
// 	for (size_t i=0, ie=cmd.m_tvs.size(); i < ie; ++i)
// 	{
// 		if (cmd.m_tvs[i].m_tag == tlv::tag_file) // ehm, actually it's a dir
// 		{
// 			onExportDataToCSV(cmd.m_tvs[i].m_val);
// 			return true;
// 		}
// 	}
// 	return false;
// }

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


