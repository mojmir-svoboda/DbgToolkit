#include "connection.h"
#include <QtNetwork>
#include <QCheckBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QFile>
#include <QDataStream>
#include "modelview.h"
#include <boost/tokenizer.hpp>
#include "../tlv_parser/tlv_encoder.h"

Connection::Connection (QObject * parent)
	: QTcpSocket(parent)
	, m_main_window(0)
	, m_app_idx(-1)
	, m_tab_idx(-2)
	, m_from_file(false)
	, m_table_view_widget(0)
	, m_tree_view_file_model(0)
	, m_tree_view_func_model(0)
	, m_columns_setup(0)
	, m_buffer(e_ringbuff_size)
	, m_current_cmd()
	, m_decoder()
	, m_name()
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
	if (i != m_tab_idx)
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
		if (e.Commit())
			write(e.buffer, e.total_len);
	}
}

void Connection::setupColumns (QList<QString> * cs)
{
	m_columns_setup = cs;
	m_tags2columns.clear();
	for (size_t i = 0, ie = cs->size(); i < ie; ++i)
	{
		size_t const tag_idx = tlv::tag_for_name(cs->at(i).toStdString().c_str());
		if (tag_idx != tlv::tag_invalid)
		{
			m_tags2columns.insert(tag_idx, static_cast<int>(i)); // column index is int in Qt toolkit
			//m_columns2tags[i] = tag_idx;
			m_table_view_widget->model()->insertColumn(i);
			qDebug("Connection::setupColumns col[%u] tag_idx=%u tag_name=%s", i, tag_idx, cs->at(i).toStdString().c_str());
		}
	}
	m_current_cmd.tvs.reserve(cs->size());
}

void Connection::setupThreadColors (QList<QColor> const & tc)
{
	m_thread_colors = tc;
}

int Connection::findColumn4Tag (tlv::tag_t tag) const
{
	QMap<tlv::tag_t, int>::const_iterator it = m_tags2columns.find(tag);
	if (it != m_tags2columns.end())
		return it.value();
	return -1;
}
void Connection::insertColumn4Tag (tlv::tag_t tag, int column_idx)
{
	m_tags2columns.insert(tag, column_idx);
}
void Connection::insertColumn ()
{
	m_columns_setup->push_back(QString());
	m_main_window->getColumnSizes(m_app_idx).push_back(127);
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

inline void Dump (DecodedCommand const & c)
{
#ifdef _DEBUG
	qDebug("command.hdr.cmd = 0x%x command.hdr.len = 0x%x", c.hdr.cmd, c.hdr.len);
	for (size_t i = 0; i < c.tvs.size(); ++i)
		qDebug("tlv[%u] t=%02x val=%s", i, c.tvs[i].m_tag, c.tvs[i].m_val.c_str());
#endif
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
			//qDebug("RCV: readed=%u free=%u to_read=%u avail=%u", count, free_space, to_read, bytesAvailable());
			if (count <= 0)
				break;	// no more data in socket

			for (size_t i = 0; i < count; ++i)
				m_buffer.push_back(local_buff[i]);
		}

		static_cast<ModelView *>(m_table_view_widget->model())->transactionStart();
		// try process data in ring buffer
		while (1)
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
					//Dump(m_current_cmd);
					//qDebug("CONN: hdr_sz=%u payload_sz=%u buff_sz=%u ",  tlv::Header::e_Size, m_current_cmd.hdr.len, m_buffer.size());
					tryHandleCommand(m_current_cmd);
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

		// @TODO: scoped guard
		static_cast<ModelView *>(m_table_view_widget->model())->transactionCommit();
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
	processStream(static_cast<QIODevice *>(this), &QTcpSocket::read);
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

/*	{	// hotfix for resizeSection on empty table .. perhaps due to transaction_begin, transaction end?
		static bool first = true;
		if (first)
		{*/
			MainWindow::columns_sizes_t const & sizes = m_main_window->getColumnSizes(m_app_idx);
			for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
			{
				m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
			}
/*			first = false;
		}
	}*/

	if (!m_from_file && m_datastream) // @TODO: && persistenCheckBox == true
		m_datastream->writeRawData(&cmd.orig_message[0], cmd.hdr.len + tlv::Header::e_Size);
	return true;
}

bool Connection::handleSetupCommand (DecodedCommand const & cmd)
{
	qDebug("handle setup command");
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
	{
		if (cmd.tvs[i].m_tag == tlv::tag_app)
		{
			QString app_name = QString::fromStdString(cmd.tvs[i].m_val);
			m_name = app_name;
			m_main_window->getTabTrace()->setTabText(m_tab_idx, app_name + QString::number(m_tab_idx));
			QString storage_name = createStorageName();
			setupStorage(storage_name);

			m_app_idx = m_main_window->findAppName(app_name);
			m_columns_setup = &m_main_window->getColumnSetup(m_app_idx);
			if (m_main_window->getColumnSetup(m_app_idx).size())	// load if config already exists
			{
				setupColumns(&m_main_window->getColumnSetup(m_app_idx));
				/*MainWindow::columns_sizes_t const & sizes = m_main_window->getColumnSizes(m_app_idx);
				for (size_t c = 0, ce = sizes.size(); c < ce; ++c)
				{
					m_table_view_widget->horizontalHeader()->resizeSection(c, sizes.at(c));
				}*/
			}

		}
	}
	return true;
}

bool Connection::handleLogCommand (DecodedCommand const & cmd)
{
	appendToFilters(cmd);
	bool excluded = false;

	if (cmd.hdr.cmd == tlv::cmd_scope_entry || (cmd.hdr.cmd == tlv::cmd_scope_exit))
	{
		// @TODO: hack na nefungujici scopes
		//static_cast<ModelView *>(m_table_view_widget->model())->appendCommand(cmd, excluded);
		if (!m_main_window->scopesEnabled())
		{
			excluded = true;
		}
		if (excluded) //@TODO: hack dup!
		{
			//m_table_view_widget->hideRow(m_table_view_widget->model()->rowCount() - 1);
		}
	}
	else if (cmd.hdr.cmd == tlv::cmd_log)
	{
		static_cast<ModelView *>(m_table_view_widget->model())->appendCommand(cmd, excluded);
		if (excluded)
		{
			m_table_view_widget->hideRow(m_table_view_widget->model()->rowCount() - 1);
		}
	}
	return true;
}

//////////////////// filtering stuff //////////////////////////////
void Connection::setupModelFile ()
{
	m_tree_view_file_model = new QStandardItemModel;
	m_main_window->getTreeViewFile()->setModel(m_tree_view_file_model);
	//main_window->getTreeViewFile()->expandAll();
}

void Connection::onApplyFilterClicked ()
{
	//excluded = m_connection->isFileLineExcluded(std::make_pair(file, line));
	//m_table_view_widget->hideRow(m_table_view_widget->model()->rowCount() - 1);
}

inline QList<QStandardItem *> addRow (QString const & str)
{
	QList<QStandardItem *> row_items;
	QStandardItem * name_item = new QStandardItem(str);
	row_items << name_item;
	return row_items;
}

QStandardItem * findChildByText (QStandardItem * parent, QString const & txt)
{
	for (size_t i = 0, ie = parent->rowCount(); i < ie; ++i)
	{
		if (parent->child(i)->text() == txt)
			return parent->child(i);
	}
	return 0;
}

bool Connection::isFileLineExcluded (fileline_t const & item)
{
	return m_file_filters.is_excluded(item.first + "/" + item.second);
}

void Connection::appendFileFilter (fileline_t const & item)
{
	m_file_filters.append(item.first + "/" + item.second);
}

void Connection::removeFileFilter (fileline_t const & item)
{
	m_file_filters.exclude_off(item.first + "/" + item.second);
	//@TODO: if removing (file:.*), call showRows on corresponding lines! (or on Apply button click?)
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
			int idx = m_tls.findThreadId(cmd.tvs[i].m_val);
			if (cmd.hdr.cmd == tlv::cmd_scope_entry)
				m_tls.incrIndent(idx);
			if (cmd.hdr.cmd == tlv::cmd_scope_exit)
				m_tls.decrIndent(idx);
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
	return m_name + QString::number(m_tab_idx);
}

bool Connection::setupStorage (QString const & name)
{
	if (!m_from_file && !m_storage)
	{
		m_storage = new QFile(name + ".tlv_trace");
		m_storage->open(QIODevice::WriteOnly);
		m_datastream = new QDataStream(m_storage);	 // we will serialize the data into the file

		if (!m_datastream)
		{
			return false;
		}
	}
	return true;
}

void Connection::exportStorageTo (QString const & filename)
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


