#include <trace_client/trace.h>
#include <cmath>

#include <cstdio>
#include <cstdlib>
#include "thrpool.h"
#include "foo.h"
#include "bar.h"

void sleep_ms (int ms)
{
#if defined WIN32 || defined WIN64
	Sleep(ms);
#elif defined __linux__
	usleep(ms * 1000);
#endif
}

void my_custom_vaarg_fn (char const * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	TRACE_MSG_VA(LL_NORMAL, CTX_Default, fmt, args);
	va_end(args);
}

unsigned g_Quit = 0;

#if defined WIN32 || defined WIN64
DWORD WINAPI thread_func ( LPVOID )
#elif defined __linux__
void * thread_func ( void * )
#endif
{
	TRACE_SCOPE(LL_NORMAL, CTX_Default);
	while (!g_Quit)
	{
		static int i = 0;
		++i;
		TRACE_MSG(LL_NORMAL, CTX_Default,  "Thread tick i=%u", i);
		sleep_ms(100);
	}
	return 0;
}

static int g_ctrl = 0;

#if defined WIN32 || defined WIN64
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
int main ()
#elif defined __linux__
int main ()
#endif
{
#if defined WIN32
	setvbuf(stdout, 0, _IONBF, 0);
#endif
	TRACE_APPNAME("Simple Client");
	TRACE_SET_LEVEL(CTX_Other | CTX_Main | CTX_Default, LL_NORMAL | LL_ERROR | LL_FATAL | LL_WARNING);
	trace::DictionaryPair lvl_dict[] { {LL_VERBOSE, "Verbose"}, {LL_DEBUG, "Debug"}, {LL_NORMAL, "Normal"}, {LL_WARNING, "Warn"}, {LL_ERROR, "Error"}, {LL_FATAL, "Fatal"} };
	TRACE_SET_LEVEL_DICTIONARY(lvl_dict, sizeof(lvl_dict) / sizeof(*lvl_dict));
	trace::DictionaryPair ctx_dict[]{ { CTX_Default, "Default" },{ CTX_Main, "Main" },{ CTX_Render, "Render" },{ CTX_Other, "Other" }};
	TRACE_SET_CONTEXT_DICTIONARY(ctx_dict, sizeof(ctx_dict) / sizeof(*ctx_dict));

	TRACE_CONNECT("localhost", "13127");

	TRACE_SCOPE(LL_NORMAL, CTX_Default);
	TRACE_MSG(LL_NORMAL, CTX_Default,  "Text with \"Error\" keyword inside");
	TRACE_MSG(LL_NORMAL, CTX_Default, "Text with \"Error\" keyword inside");
	TRACE_MSG(LL_FATAL, CTX_Default,  "fatal error");
	TRACE_MSG(LL_WARNING, CTX_Default, "some warn");
	TRACE_MSG(LL_WARNING, CTX_Default, "dong");
	TRACE_MSG(LL_WARNING, CTX_Default, "some warn.. and a DING!");
	TRACE_MSG(LL_VERBOSE, CTX_Default, "None shall pass!!!");


	{
		TRACE_MSG(LL_WARNING, CTX_Default,	"first message\nsecond line");
		TRACE_MSG(LL_WARNING, CTX_Main,	"first game message\nsecond line");
		TRACE_MSG(LL_ERROR, CTX_Main,	"first game error\nsecond line");
		TRACE_MSG(LL_ERROR, CTX_Main,	"second game error\nsecond line of that");
		TRACE_MSG(LL_WARNING, CTX_Default,  "First warning, errno=0x%08x", 0xFEEDDEAD);
		TRACE_MSG(LL_WARNING, CTX_Default,  "Second warning, errno=0x%08x", 0xFEEDDEAD);

		TRACE_MSG(LL_WARNING, CTX_Default, "Alice opened the door and found that it led into a small passage, not much larger than a rat - hole: she knelt down and looked along the passage into the loveliest garden you ever saw.How she longed to get out of that dark hall, and wander about among those beds of bright flowers and those cool fountains, but she could not even get her head through the doorway; \'and even if my head would go through,\' thought poor Alice, \'it would be of very little use without my shoulders. Oh, how I wish I could shut up like a telescope! I think I could, if I only knew how to begin.\' For, you see, so many out - of - the - way things had happened lately, that Alice had begun to think that very few things indeed were really impossible. There seemed to be no use in waiting by the little door, so she went back to the table, half hoping she might find another key on it, or at any rate a book of rules for shutting people up like telescopes : this time she found a little bottle on it, (\'which certainly was not here before,\' said Alice, ) and round the neck of the bottle was a paper label, with the words \'DRINK ME\' beautifully printed on it in large letters.\
			It was all very well to say \'Drink me,\' but the wise little Alice was not going to do that in a hurry. \'No, I\'ll look first, \' she said, \'and see whether it\'s marked \"poison\" or not\'; for she had read several nice little histories about children who had got burnt, and eaten up by wild beasts and other unpleasant things, all because they would not remember the simple rules their friends had taught them : such as, that a red - hot poker will burn you if you hold it too long; and that if you cut your finger very deeply with a knife, it usually bleeds; and she had never forgotten that, if you drink much from a bottle marked \'poison,\' it is almost certain to disagree with you, sooner or later.\
			However, this bottle was not marked \'poison,\' so Alice ventured to taste it, and finding it very nice, (it had, in fact, a sort of mixed flavour of cherry - tart, custard, pine - apple, roast turkey, toffee, and hot buttered toast, ) she very soon finished it off.");

		foo();
		Bar bar;

		for (int i = 0; i < 64; ++i)
		{
			float x = 3.1415926535f * 2.0f / 128.0f * static_cast<float>(i);
			TRACE_PLOT_XY(LL_NORMAL, CTX_Default, x, sinf(x), "sample_plot/sin(x)");
			TRACE_PLOT_XY(LL_NORMAL, CTX_Default, x, cosf(x), "sample_plot/cos(x)");
			if (i == 31)
				TRACE_PLOT_XY_MARKER(LL_NORMAL, CTX_Default, x, cosf(x), "sample_plot/sin(x)");
			if (i == 63)
				TRACE_PLOT_XY_MARKER(LL_NORMAL, CTX_Default, x, cosf(x), "sample_plot/m1");
			TRACE_MSG(LL_NORMAL, CTX_Default, "sample_plot i = %i", i);
		}

		//ThreadPool<2> thr_pool;
		//thr_pool.Create(thread_func, 0);
		//TRACE_SOUND(LL_NORMAL, CTX_Default, 0.5f, 1, "Tech-01");
			
		int s = 0;
		for (;;)
		{
			static int i = 0;
			s += 100;
			
			sleep_ms(s);
			//sleep_ms(1000);
			//sleep_ms(1000);

			TRACE_MSG(LL_NORMAL, CTX_Default,  "normal message i=%u from main thread", i);
			TRACE_MSG(LL_VERBOSE, CTX_Default,  "verbose message i=%u from main thread", i);
			TRACE_MSG(LL_DEBUG, CTX_Default, "debug message i=%u from main thread", i);

			TRACE_MSG(LL_VERBOSE, CTX_Render, "verbose message i=%u from render subsys", i);
			TRACE_MSG(LL_DEBUG, CTX_Render, "debug message i=%u from render subsys", i);
			TRACE_MSG(LL_WARNING, CTX_Render, "warn message i=%u from render subsys", i);
			TRACE_MSG(LL_ERROR, CTX_Render, "err message i=%u from render subsys", i);

			if (i == 10)
				TRACE_MSG(LL_NORMAL, CTX_Default, "some warn.. and BAD DONG!");
			//TRACE_SOUND(LL_NORMAL, CTX_Default, 0.5f, 1, "Tech-02");
			//TRACE_SOUND(LL_NORMAL, CTX_Default, 0.5f, 1, "Tech-01");
      //TRACE_MSG(LL_DEBUG, CTX_Default,  "Some detailed message i=%u from main thread", i);
			++i;
			if (i == 5000)
				break;
		}
		g_Quit = 1;
		//thr_pool.WaitForTerminate();

		for (;;)
			;
	}

	TRACE_DISCONNECT();
}

