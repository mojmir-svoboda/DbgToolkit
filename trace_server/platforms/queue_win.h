#pragma once
#include <cstring>
#include <cstdio>
#include <cstdlib>
//#include <unistd.h>
#include <errno.h>
#include <ctype.h>
//#include <stdint.h>

#if defined WIN32 || defined __MINGW__
//|| defined WIN64
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#else

	inline void atomic_write32 (volatile uint32_t * mem, uint32_t val)
	{
		*mem = val;
	}

#	define InterlockedExchangePointer(dst, val)	\
		atomic_write32((uint32_t *)(dst), (uintptr_t)(val))
#endif


// Code from Herb Sutter's Oct 2008 article ( http://www.ddj.com/hpc-high-performance-computing/210604448 )
template <typename T>
class LFQueue
{
	struct Node {
		T       m_Value;
		Node *  m_Next;

		Node (T t) : m_Value(t), m_Next(0) { }
	};

	Node * m_First;		// Producer Only
	Node * m_Divider;	// Producer / Consumer
	Node * m_Last;		// Producer / Consumer

public:
	LFQueue ()
	{
		m_First = m_Divider = m_Last = new Node(T());
	}

	~LFQueue()
	{
		while (m_First)
		{
			Node * temp = m_First;
			m_First = temp->m_Next;
			delete temp;
		}
	}

	void Produce (T const & rNew)
	{
		m_Last->m_Next = new Node(rNew);
		//m_Last = m_Last->m_Next;
		InterlockedExchangePointer(reinterpret_cast<PVOID volatile *>(&m_Last), m_Last->m_Next);

		while (m_First != m_Divider)
		{
			Node * temp = m_First;
			m_First = m_First->m_Next;
			delete temp;
		}
	}

	bool Consume (T & result)
    {
		if (m_Divider != m_Last)
		{
			result = m_Divider->m_Next->m_Value;
			//m_Divider = m_Divider->m_Next;
			InterlockedExchangePointer(reinterpret_cast<PVOID volatile *>(&m_Divider), m_Divider->m_Next);
			return true;
		}
		return false;
	}
};

#undef InterlockedExchangePointer
/*template <typename T>
class LFQueue
{

public:
	std::vector<T> m_items;
	LFQueue () { }
	~LFQueue() { }
	void Produce (T const & rNew) { m_items.push_back(rNew); }

	bool Consume (T & result)
	{
		if (m_items.empty())
			return false;

		result = m_items.back();
		m_items.pop_back();
		return true;
	}
};*/


