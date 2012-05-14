#pragma once
#include "queue_posix.h"

template<typename T>
struct RendezVous
{
	typedef LFQueue<T *> T_Queue;
	T_Queue m_Queue;

	void Produce (T * t)
	{
		m_Queue.Produce(t);
	}

	bool Consume (T * & t)
	{
		t = 0;
		return m_Queue.Consume(t);
	}
};

