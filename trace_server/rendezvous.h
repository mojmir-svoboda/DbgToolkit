#pragma once
#if defined WIN32
#	include "platforms/queue_win.h"
#else
#	include "platforms/queue_posix.h"
#endif
#include <vector>

template<typename T, class SourceT, class TargetT>
struct RendezVousPoint
{
	typedef LFQueue<T *> T_Queue;
	typedef SourceT T_Source;
	typedef TargetT T_Target;
	T_Queue * m_Queue;
	T_Source m_Source;
	T_Target m_Target;

	RendezVousPoint ()
		: m_Queue(new T_Queue)
		, m_Source(), m_Target()
	{ }

	~RendezVousPoint ()
	{
		delete m_Queue;
		m_Queue = 0;
	}

	void produce (T * t)
	{
		m_Queue->Produce(t);
	}

	/*bool consume (T const * & t)
	{
		t = 0;
		return m_Queue->Consume(t);
	}*/
	bool consume (T * & t)
	{
		t = 0;
		return m_Queue->Consume(t);
	}

};

template<typename T, class SourceT, class TargetT>
struct RendezVouses : std::vector<T *> {
	typedef RendezVousPoint<T *, SourceT, TargetT> rvp_t;
	typedef std::vector<T *> rvps_t;

	RendezVouses ()
	{
		reserve(16);
	}

	T * create ()
	{
		push_back(new T);
		return back();
	}

	void destroy (T const * item)
	{
		erase(std::remove(begin(), end(), item), end());
		delete item;
	}
};

