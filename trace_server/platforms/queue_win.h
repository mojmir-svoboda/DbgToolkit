#pragma once
#include <cstring>
#include <cstdio>
#include <sysfn/select_atomic.h>

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
		sys::atomic_wrtptr(reinterpret_cast<PVOID volatile *>(&m_Last), m_Last->m_Next); //m_Last = m_Last->m_Next;

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
			
			sys::atomic_wrtptr(reinterpret_cast<PVOID volatile *>(&m_Divider), m_Divider->m_Next); // m_Divider = m_Divider->m_Next
			return true;
		}
		return false;
	}
};

