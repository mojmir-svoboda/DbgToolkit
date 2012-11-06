#include "profilerconnection.h"
#include "profilerserver.h"
#include <boost/tokenizer.hpp>
#include <tlv_parser/tlv_encoder.h>
#include <QtDebug>
#include "mainwindow.h"


namespace profiler {

	Server::Server (boost::asio::io_service & io_service, boost::asio::ip::tcp::endpoint const & endpoint, MainWindow & mw)
		: m_io_service(io_service)
		, m_acceptor(io_service, endpoint)
		, m_main_window(mw)
	{
		start_accept();
	}

	void Server::start_accept ()
	{
		profiler_rvp_t * const rvp = getRendezVouses().create();
		connection_ptr_t new_session(new Connection(NULL, m_io_service, *rvp, m_main_window));
		rvp->m_Source = new_session;

		m_acceptor.async_accept(new_session->socket(),
				boost::bind(&Server::handle_accept, this, new_session, boost::asio::placeholders::error));
	}

	void Server::handle_accept (connection_ptr_t session, boost::system::error_code const & error)
	{
		if (!error)
			session->start();
		start_accept();
	}
}

Q_DECLARE_METATYPE(profiler::profiler_rvp_t *)

namespace profiler {

Connection::Connection (QObject * parent, boost::asio::io_service & io_service, profiler_rvp_t & rvp, MainWindow & mw)
	: QObject(parent)
	, m_io(io_service), m_profileInfo(), m_socket(io_service) , m_current_cmd() , m_decoder()
	, m_rvp(rvp)
	, m_last_flush_end_idx(0)
	, m_main_window(mw)
{
	qRegisterMetaType<profiler::profiler_rvp_t *>("prof_rvp_t");
	connect(this, SIGNAL(incomingProfilerConnection(profiler::profiler_rvp_t *)), m_main_window.getServer(), SLOT(incomingProfilerConnection(profiler::profiler_rvp_t *)), Qt::QueuedConnection);
	//printf("+++ connection\n");
}

void Connection::start ()
{
	emit incomingProfilerConnection(&m_rvp);
	boost::asio::async_read(m_socket,
		boost::asio::buffer(&m_current_cmd.orig_message[0], tlv::Header::e_Size),
		boost::bind(&Connection::handle_read_header, shared_from_this(), boost::asio::placeholders::error));
}


bool Connection::tryHandleCommand (DecodedCommand const & cmd)
{
	if (cmd.hdr.cmd == tlv::cmd_setup)
		handleSetupCommand(cmd);
	else if (cmd.hdr.cmd == tlv::cmd_profile_bgn || cmd.hdr.cmd == tlv::cmd_profile_end || cmd.hdr.cmd == tlv::cmd_profile_frame_bgn || cmd.hdr.cmd == tlv::cmd_profile_frame_end)
		handleProfileCommand(cmd);
	return true;
}

bool Connection::handleSetupCommand (DecodedCommand const & cmd)
{
	printf("handle setup command\n");
	for (size_t i=0, ie=cmd.tvs.size(); i < ie; ++i)
		if (cmd.tvs[i].m_tag == tlv::tag_app)
			printf("received setup command from application: %s\n", cmd.tvs[i].m_val.toStdString().c_str());

	m_profileInfo.m_completed_frame_infos.push_back(new threadinfos_t()); // @TODO: reserve
	return true;
}

void dumpCommand (DecodedCommand const & cmd)
{
	printf("cmd=0x%02X||", cmd.hdr.cmd);
	char const * const fmts[tlv::tag_max_value] = {
		"%s|",
		"app=%s|",
		"pid=%s|",
		"tim=%-10s|",
		"tid=%-10s|",
		"fil=%-32s|",
		"lin=%-6s|",
		"fun=%-32s|",
		"msg=%s",
		"lvl=%-2s|",
		"ctx=%-8s|" };
	for (size_t i = 0, ie = cmd.tvs.size(); i < ie; ++i)
	{
		tlv::tag_t const tag = cmd.tvs[i].m_tag;
		QString const & val = cmd.tvs[i].m_val;
		//printf(fmts[tag], val.c_str());
	}
	printf("\n"); fflush(stdout);
}

bool Connection::handleProfileCommand (DecodedCommand const & cmd)
{
	long long time = 0;
	long long tid = 0;
	QString text;
	unsigned tid_idx = 0;

	//printf("incoming: "); dumpCommand(cmd);

	for (size_t i = 0, ie = cmd.tvs.size(); i < ie; ++i)
	{
		tlv::tag_t const tag = cmd.tvs[i].m_tag;
		QString const & val = cmd.tvs[i].m_val;

		if (cmd.tvs[i].m_tag == tlv::tag_tid)
		{
#ifdef WIN32
			tid  = val.toULongLong();
#else
			tid  = val.toUnsignedLongLong();
#endif
			std::vector<unsigned long long>::iterator it = std::find(m_profileInfo.m_tids.begin(), m_profileInfo.m_tids.end(), tid);
			if (it == m_profileInfo.m_tids.end())
			{
				tid_idx = m_profileInfo.m_tids.size();
				m_profileInfo.m_tids.push_back(tid);

				m_profileInfo.m_completed_frame_infos[m_profileInfo.m_frame]->push_back(blockinfos_t()); // @TODO: reserve

				m_profileInfo.m_pending_infos.push_back(blockinfos_t());
			}
			else
			{
				tid_idx = std::distance(m_profileInfo.m_tids.begin(), it);
			}
		}

		if (cmd.tvs[i].m_tag == tlv::tag_time)
#ifdef WIN32
			time = val.toULongLong();
#else
			time = atoll(val.c_str());
#endif

		if (cmd.tvs[i].m_tag == tlv::tag_msg)
			text = val;
	}

	if (cmd.hdr.cmd == tlv::cmd_profile_frame_bgn)
	{
		m_profileInfo.m_frame_begin = time;
		m_profileInfo.m_completed_frame_infos.push_back(new threadinfos_t()); // @TODO: reserve

		for(size_t i = 0, ie = m_profileInfo.m_tids.size(); i < ie; ++i)
		{
			m_profileInfo.m_completed_frame_infos.back()->push_back(blockinfos_t()); // @TODO: reserve
		}

		++m_profileInfo.m_frame;
		return true;
	}

	if (cmd.hdr.cmd == tlv::cmd_profile_frame_end)
	{
		m_profileInfo.m_frames.push_back(std::make_pair(m_profileInfo.m_frame_begin / g_scaleValue, time / g_scaleValue));
	
		size_t const from = m_last_flush_end_idx;
		size_t const to = m_profileInfo.m_completed_frame_infos.size();
		//qDebug("flushing from %i to %i", from, to);

		for (size_t i = from; i < to; ++i)
		{
			//qDebug("producing item=0x%016x %i, sz=%u", m_profileInfo.m_completed_frame_infos[i], i, m_profileInfo.m_completed_frame_infos[i]->size());
			m_rvp.produce(m_profileInfo.m_completed_frame_infos[i]);
		}

		//dump
		/*for (size_t i = from; i < to; ++i)
			for (size_t j = 0, je = m_profileInfo.m_completed_frame_infos[i]->size(); j < je; ++j)
				qDebug("producing item[%i]=0x%016x, tis=%u bis_sz=%u", i, m_profileInfo.m_completed_frame_infos[i], m_profileInfo.m_completed_frame_infos[i]->size(),  m_profileInfo.m_completed_frame_infos[i]->operator[](j).size());
*/
		m_last_flush_end_idx = to;
		emit incomingProfilerData(&m_rvp);
		return true;
	}

	if (cmd.hdr.cmd == tlv::cmd_profile_bgn)
	{
		BlockInfo * prev = 0;
		if (m_profileInfo.m_pending_infos[tid_idx].size())
			prev = m_profileInfo.m_pending_infos[tid_idx].back();
		m_profileInfo.m_bi_ptrs.push_back(new BlockInfo());
		m_profileInfo.m_pending_infos[tid_idx].push_back(m_profileInfo.m_bi_ptrs.back());
		BlockInfo & bi = *m_profileInfo.m_pending_infos[tid_idx].back();
		bi.m_time_bgn = time;
		bi.m_tid = tid;
		bi.m_tid_idx = tid_idx;
		bi.m_msg = text;
		bi.m_layer = m_profileInfo.m_pending_infos[tid_idx].size() - 1;
		bi.m_frame = m_profileInfo.m_frame;
		bi.m_parent = prev;
	}
	else if (cmd.hdr.cmd == tlv::cmd_profile_end)
	{
		unsigned const frame_idx = m_profileInfo.m_frame;

		BlockInfo * bi = m_profileInfo.m_pending_infos[tid_idx].back();
		m_profileInfo.m_pending_infos[tid_idx].pop_back();

		bi->m_time_end = time;
		bi->m_msg.append(text);
		bi->complete();

		(*m_profileInfo.m_completed_frame_infos[bi->m_frame])[tid_idx].push_back(bi);
	}

	return true;
}

} // namespace profiler

