#include <pthread.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct thread_info {    /* Used as argument to thread_start() */
	pthread_t thread_id;        /* ID returned by pthread_create() */
	int       thread_num;       /* Application-defined thread # */
	char     *argv_string;      /* From command-line argument */
};

/* Thread start function: display address near top of our stack, and return upper-cased copy of argv_string */

static void * thread_start (void * arg)
{
	thread_info * tinfo = static_cast<thread_info *>(arg);

	char *p = 0;
	printf("+++ Thread %d: top of stack near %p; argv_string=\n", tinfo->thread_num, &p/*, tinfo->argv_string*/);

	long i = 0;
	while (1)
	{
		if (++i > 1e8)
			break;
	}
	printf("--- Thread %d: top of stack near %p; argv_string=\n", tinfo->thread_num, &p/*, tinfo->argv_string*/);

	return 0;
}

template<size_t N>
struct ThreadPool
{
	enum { e_thread_count = N };
	pthread_attr_t m_attr;
	thread_info m_tinfo[e_thread_count];

	ThreadPool () { memset(this, 0, sizeof(*this)); }
	~ThreadPool () { Close(); }
	void Create (void * (* fn) (void *), void * )
	{
		/* Initialize thread creation attributes */
		int s = pthread_attr_init(&m_attr);
		if (s != 0)
			handle_error_en(s, "pthread_attr_init");
		/*if (stack_size > 0)
		{
			s = pthread_attr_setstacksize(&m_attr, stack_size);
			if (s != 0)
				handle_error_en(s, "pthread_attr_setstacksize");
		}*/

		/* Create one thread for each command-line argument */
		for (size_t t = 0; t < N; ++t)
		{
			m_tinfo[t].thread_num = t + 1;
			//m_tinfo[t].argv_string = argv[optind + t];

			cpu_set_t cpuset;

			CPU_ZERO(&cpuset);
			CPU_SET(t, &cpuset);
			//CPU_SET(atoi(m_tinfo[t].argv_string), &cpuset);
			pthread_attr_setaffinity_np(&m_attr, sizeof(cpuset), &cpuset);

			/* The pthread_create() call stores the thread ID into corresponding element of m_tinfo[] */
			s = pthread_create(&m_tinfo[t].thread_id, &m_attr, fn, &m_tinfo[t]);
			if (s != 0)
				handle_error_en(s, "pthread_create");
		}

	}
	void WaitForTerminate ()
	{
		/* Destroy the thread attributes object, since it is no longer needed */
		int s = pthread_attr_destroy(&m_attr);
		if (s != 0)
			handle_error_en(s, "pthread_attr_destroy");

		/* Now join with each thread, and display its returned value */
		for (size_t t = 0; t < N; t++)
		{
			void * res = 0;
			s = pthread_join(m_tinfo[t].thread_id, &res);
			if (s != 0)
			   handle_error_en(s, "pthread_join");

			printf("Joined with thread %d; returned value was %s\n", m_tinfo[t].thread_num, (char *) res);
			free(res);      /* Free memory allocated by thread */
		}

	}
	void Close ()
	{
	}
};


int main(int argc, char * argv[])
{
	printf("starting: %u\n", 4);

	ThreadPool<4> p;
	p.Create(thread_start, 0);
	;;; //////
	p.WaitForTerminate();

	exit(EXIT_SUCCESS);
}

