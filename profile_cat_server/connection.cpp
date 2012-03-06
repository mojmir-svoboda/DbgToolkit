#include "connection.h"
#include <boost/tokenizer.hpp>
#include "../tlv_parser/tlv_encoder.h"

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
			printf("received setup command from application: %s\n", cmd.tvs[i].m_val.c_str());
	return true;
}

struct BlockInfo
{
	unsigned m_frame;
	unsigned long long m_time_bgn;
	unsigned long long m_time_end;
	unsigned long long m_delta_t;
	unsigned long long m_tid;
	std::string m_msg;

	BlockInfo () : m_frame(0), m_time_bgn(0), m_time_end(0), m_delta_t(0), m_tid(0) { }

	void complete ()
	{
		m_delta_t = m_time_end - m_time_bgn;
		printf("completed: tid=%10llu delta_t=%10llu msg=%s\n", m_tid, m_delta_t, m_msg.c_str());
	}
};

typedef std::vector<BlockInfo> blockinfos_t;
typedef std::vector<blockinfos_t> frameinfos_t;
typedef std::vector<blockinfos_t> pendinginfos_t;
typedef std::vector<frameinfos_t> threadinfos_t;

pendinginfos_t m_pending_infos;
std::vector<unsigned long long> m_tids;
unsigned m_frame = 0;
threadinfos_t m_completed_thread_infos;

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
		std::string const & val = cmd.tvs[i].m_val;
		printf(fmts[tag], val.c_str());
	}
	printf("\n"); fflush(stdout);
}

bool Connection::handleProfileCommand (DecodedCommand const & cmd)
{
	long long time = 0;
	long long tid = 0;
	std::string text;
	unsigned tid_idx = 0;

	//printf("incoming: "); dumpCommand(cmd);

	for (size_t i = 0, ie = cmd.tvs.size(); i < ie; ++i)
	{
		tlv::tag_t const tag = cmd.tvs[i].m_tag;
		std::string const & val = cmd.tvs[i].m_val;

		if (cmd.tvs[i].m_tag == tlv::tag_tid)
		{	
			tid = atoll(val.c_str());
			std::vector<unsigned long long>::iterator it = std::find(m_tids.begin(), m_tids.end(), tid);
			if (it == m_tids.end())
			{
				tid_idx = m_tids.size();
				m_tids.push_back(tid);
				m_completed_thread_infos.push_back(frameinfos_t()); // @TODO: reserve
				m_completed_thread_infos[tid_idx].push_back(blockinfos_t()); // @TODO: reserve
				m_pending_infos.push_back(blockinfos_t());
			}
			else
			{
				tid_idx = std::distance(m_tids.begin(), it);
			}
		}

		if (cmd.tvs[i].m_tag == tlv::tag_time)
			time = atoll(val.c_str());

		if (cmd.tvs[i].m_tag == tlv::tag_msg)
			text = val;
	}

	if (cmd.hdr.cmd == tlv::cmd_profile_frame_bgn)
	{
		printf("+++ frame begin\n");
		return true;
	}

	if (cmd.hdr.cmd == tlv::cmd_profile_frame_end)
	{
		printf("--- frame end\n");
		++m_frame;

		for(size_t i = 0, ie = m_tids.size(); i < ie; ++i)
		{
			m_completed_thread_infos[i].push_back(blockinfos_t()); // @TODO: reserve
		}
		return true;
	}

	if (cmd.hdr.cmd == tlv::cmd_profile_bgn)
	{
		m_pending_infos[tid_idx].push_back(BlockInfo());
		BlockInfo & bi = m_pending_infos[tid_idx].back();
		bi.m_time_bgn = time;
		bi.m_tid = tid;
		bi.m_msg = text;
		bi.m_frame = m_frame;
	}
	else if (cmd.hdr.cmd == tlv::cmd_profile_end)
	{
		unsigned const frame_idx = m_frame;

		BlockInfo bi = m_pending_infos[tid_idx].back();
		m_pending_infos[tid_idx].pop_back();

		bi.m_time_end = time;
		bi.complete();
		m_completed_thread_infos[tid_idx][bi.m_frame].push_back(bi);
	}
	//printf("\n"); fflush(stdout);

	return true;
}

