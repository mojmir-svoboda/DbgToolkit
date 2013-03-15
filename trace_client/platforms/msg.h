#pragma once
#include <sysfn/macros.h>
#include <sysfn/select_atomic.h>

namespace sys {

	/**@class	Message
	 * @brief	storage for message to be logged
	 *
	 * Message features 4-phase lifetime:
	 *	a) e_clean    --> e_writing
	 *		some thread is writing now in the buffer
	 *	b) e_writing  --> e_dirty
	 *		data are written and ready to be flushed
	 *	c) e_dirty    --> e_flushing
	 *	    consumer thread is flushing into storage or network
	 *	d) e_flushing --> e_clean
	 *		data are flushed and buffer can be reused
	 */
	template <unsigned N = 192>
	struct Message
	{
		CACHE_ALIGN 		atomic32_t mutable volatile m_lock;
		unsigned m_length;
		enum { e_data_sz = N - sizeof(atomic32_t) - sizeof(unsigned) };
		char m_data[e_data_sz];

		enum {
			e_clean    = 0,
			e_writing  = 1,
			e_dirty    = 2,
			e_flushing = 3,
		};

		void WriteUnlockAndDirty ()
		{
			atomic_cas32(&m_lock, e_writing, e_dirty);
		}

		void WriteLock ()
		{
			for (;;)
			{
				atomic32_t prev_lock = atomic_cas32(&m_lock, e_clean, e_writing);
				if (m_lock == e_writing && prev_lock == e_clean)
					break;
				else
					thread_yield();
			}
		}

		void ReadLock ()
		{
			for (;;)
			{
				atomic32_t prev_lock = atomic_cas32(&m_lock, e_dirty, e_flushing);
				if (m_lock == e_flushing && prev_lock == e_dirty)
					break;
				else
					thread_yield();
			}
		}

		void ReadUnlockAndClean ()
		{
			atomic_cas32(&m_lock, e_flushing, e_clean);
		}
	};
}

