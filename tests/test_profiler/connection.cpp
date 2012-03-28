#include "connection.h"
#include <boost/tokenizer.hpp>
#include <tlv_parser/tlv_encoder.h>

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

	m_profileInfo.m_completed_frame_infos.push_back(threadinfos_t()); // @TODO: reserve
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
#ifdef WIN32
			tid  = _atol_l(val.c_str(), 0);
#else
			tid  = atoll(val.c_str());
#endif
			std::vector<unsigned long long>::iterator it = std::find(m_profileInfo.m_tids.begin(), m_profileInfo.m_tids.end(), tid);
			if (it == m_profileInfo.m_tids.end())
			{
				tid_idx = m_profileInfo.m_tids.size();
				m_profileInfo.m_tids.push_back(tid);

				m_profileInfo.m_completed_frame_infos[m_profileInfo.m_frame].push_back(blockinfos_t()); // @TODO: reserve

				m_profileInfo.m_pending_infos.push_back(blockinfos_t());
			}
			else
			{
				tid_idx = std::distance(m_profileInfo.m_tids.begin(), it);
			}
		}

		if (cmd.tvs[i].m_tag == tlv::tag_time)
#ifdef WIN32
			time = _atol_l(val.c_str(), 0);
#else
			time = atoll(val.c_str());
#endif

		if (cmd.tvs[i].m_tag == tlv::tag_msg)
			text = val;
	}

	if (cmd.hdr.cmd == tlv::cmd_profile_frame_bgn)
	{
		m_profileInfo.m_frame_begin = time;
		m_profileInfo.m_completed_frame_infos.push_back(threadinfos_t()); // @TODO: reserve

		for(size_t i = 0, ie = m_profileInfo.m_tids.size(); i < ie; ++i)
		{
			m_profileInfo.m_completed_frame_infos.back().push_back(blockinfos_t()); // @TODO: reserve
		}

		++m_profileInfo.m_frame;
		return true;
	}

	if (cmd.hdr.cmd == tlv::cmd_profile_frame_end)
	{
		m_profileInfo.m_frames.push_back(std::make_pair(m_profileInfo.m_frame_begin / 1000.0f, time / 1000.0f));
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

		m_profileInfo.m_completed_frame_infos[bi->m_frame][tid_idx].push_back(bi);
	}

	return true;
}

