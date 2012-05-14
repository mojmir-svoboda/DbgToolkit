#pragma once
#include <vector>
#include <string>
#include <cstdio>

namespace profiler {

	extern float g_scaleValue;

	struct BlockInfo
	{
		unsigned m_frame;
		unsigned long long m_time_bgn;
		unsigned long long m_time_end;
		unsigned long long m_delta_t;
		unsigned long long m_tid;
		unsigned long long m_layer;
		float m_x;
		float m_y;
		float m_dt;
		std::string m_msg;
		std::string m_tag;
		BlockInfo * m_parent;

		BlockInfo () : m_frame(0), m_time_bgn(0), m_time_end(0), m_delta_t(0), m_tid(0), m_layer(0), m_parent(0) { }

		void complete ()
		{
			m_delta_t = m_time_end - m_time_bgn;
			m_dt = static_cast<float>(m_delta_t) / g_scaleValue;
			//printf("completed: tid=%10llu delta_t=%10llu msg=%s\n", m_tid, m_delta_t, m_msg.c_str());
		}
	};

	typedef std::vector<BlockInfo *> blockinfos_t;
	typedef std::vector<blockinfos_t> pendinginfos_t;

	typedef std::vector<blockinfos_t> threadinfos_t;
	typedef std::vector<threadinfos_t> frameinfos_t;

	struct ProfileInfo
	{
		ProfileInfo () : m_frame(0), m_frame_begin(0) { }

		blockinfos_t m_bi_ptrs;

		pendinginfos_t m_pending_infos;
		std::vector<unsigned long long> m_tids;
		std::vector<std::pair<float, float> > m_frames;
		std::vector<unsigned> m_critical_paths;
		unsigned m_frame;
		unsigned m_frame_begin;
		frameinfos_t m_completed_frame_infos;
	};
} // namespace profiler

