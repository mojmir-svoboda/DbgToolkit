#include <trace_client/trace.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include "foo.h"
#include "bar.h"
#include <array>
#include <windows.h>
#include "test_vaarg.h"

unsigned g_Quit = 0;

void thread_func ()
{
	TRACE_SCOPE(LL_INFO, CTX_Main);
	int j = 0;
	while (!g_Quit)
	{
		static int i = 0;
		++i; // i know, race condition.. not important
		++j;
		TRACE_MSG(LL_INFO, CTX_Main,  "Thread tick i=%u j=%u", i, j);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

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
	TRACE_INIT("SampleClient");
	TRACE_SINK_INIT(0,  "127.0.0.1", "13127");
	//TRACE_SINK_INIT(0,  "192.168.39.102", "13127");
	trace::level_t lvl_values[] { LL_SPAM, LL_VERBOSE, LL_DEBUG, LL_INFO, LL_WARNING, LL_ERROR, LL_FATAL };
	char const * lvl_names[]{ "Spam", "Verbose", "Debug", "Normal", "Warn",  "Error", "Fatal"};
	TRACE_SET_LEVEL_DICTIONARY(lvl_values, lvl_names, sizeof(lvl_values) / sizeof(*lvl_values));

	trace::context_t ctx_values[]{ CTX_Init, CTX_Main, CTX_Render, CTX_Other};
	char const * ctx_names[]{ "Init", "Main", "Render", "Other"};
	TRACE_SET_CONTEXT_DICTIONARY(ctx_values, ctx_names, sizeof(ctx_values) / sizeof(*ctx_values));

	TRACE_SINK_SET_LEVEL(0, CTX_Other | CTX_Main | CTX_Init, LL_INFO | LL_ERROR | LL_FATAL);

	TRACE_CONNECT();
	TRACE_SINK_SET_BUFFERED(0, false); // set buffered comes after fopen

	TRACE_SCOPE(LL_INFO, CTX_Main);
	TRACE_MSG(LL_INFO, CTX_Main,  "Text with \"Error\" keyword inside");
	TRACE_MSG(LL_INFO, CTX_Main, "Text with \"Error\" keyword inside");
	TRACE_MSG(LL_FATAL, CTX_Main,  "fatal error");
	TRACE_MSG(LL_WARNING, CTX_Main, "some warn");
	TRACE_MSG(LL_WARNING, CTX_Main, "dong");
	TRACE_MSG(LL_WARNING, CTX_Main, "some warn.. and a DING!");
	TRACE_MSG(LL_VERBOSE, CTX_Main, "None shall pass!!!");
	TRACE_MSG(LL_SPAM, CTX_Main, "Spam, Spam Spam Spam... Lovely Spaaam!");

	{
		TRACE_SCOPE_MSG(LL_INFO, CTX_Main, "main scope");
		TRACE_MSG(LL_WARNING, CTX_Main,	"first message\nsecond line");
		TRACE_MSG(LL_WARNING, CTX_Main,	"first game message\nsecond line");
		TRACE_MSG(LL_ERROR, CTX_Main,	"first game error\nsecond line");
		TRACE_MSG(LL_ERROR, CTX_Main,	"second game error\nsecond line of that");
		TRACE_MSG(LL_WARNING, CTX_Main,  "First warning, errno=0x%08x", 0xFEEDDEAD);
		TRACE_MSG(LL_WARNING, CTX_Main,  "Second warning, errno=0x%08x", 0xFEEDDEAD);

		TRACE_MSG(LL_WARNING, CTX_Main, "Alice opened the door and found that it led into a small passage, not much larger than a rat - hole: she knelt down and looked along the passage into the loveliest garden you ever saw.How she longed to get out of that dark hall, and wander about among those beds of bright flowers and those cool fountains, but she could not even get her head through the doorway; \'and even if my head would go through,\' thought poor Alice, \'it would be of very little use without my shoulders. Oh, how I wish I could shut up like a telescope! I think I could, if I only knew how to begin.\' For, you see, so many out - of - the - way things had happened lately, that Alice had begun to think that very few things indeed were really impossible. There seemed to be no use in waiting by the little door, so she went back to the table, half hoping she might find another key on it, or at any rate a book of rules for shutting people up like telescopes : this time she found a little bottle on it, (\'which certainly was not here before,\' said Alice, ) and round the neck of the bottle was a paper label, with the words \'DRINK ME\' beautifully printed on it in large letters.\
			It was all very well to say \'Drink me,\' but the wise little Alice was not going to do that in a hurry. \'No, I\'ll look first, \' she said, \'and see whether it\'s marked \"poison\" or not\'; for she had read several nice little histories about children who had got burnt, and eaten up by wild beasts and other unpleasant things, all because they would not remember the simple rules their friends had taught them : such as, that a red - hot poker will burn you if you hold it too long; and that if you cut your finger very deeply with a knife, it usually bleeds; and she had never forgotten that, if you drink much from a bottle marked \'poison,\' it is almost certain to disagree with you, sooner or later.\
			However, this bottle was not marked \'poison,\' so Alice ventured to taste it, and finding it very nice, (it had, in fact, a sort of mixed flavour of cherry - tart, custard, pine - apple, roast turkey, toffee, and hot buttered toast, ) she very soon finished it off.");

		foo();
		Bar bar;

// 		for (int i = 0; i < 64; ++i)
// 		{
// 			float x = 3.1415926535f * 2.0f / 128.0f * static_cast<float>(i);
// 			TRACE_PLOT_XY(LL_INFO, CTX_Render, x, sinf(x), "sample_plot/sin(x)");
// 			TRACE_PLOT_XY(LL_INFO, CTX_Render, x, cosf(x), "sample_plot/cos(x)");
// 			if (i == 31)
// 				TRACE_PLOT_XY_MARKER(LL_INFO, CTX_Render, x, cosf(x), "sample_plot/sin(x)");
// 			if (i == 63)
// 				TRACE_PLOT_XY_MARKER(LL_INFO, CTX_Render, x, cosf(x), "sample_plot/m1");
// 			TRACE_MSG(LL_INFO, CTX_Render, "sample_plot i = %i", i);
// 		}

		std::array<std::thread, 3> thrs;
		for (std::thread & t : thrs)
		{
			t = std::move(std::thread(thread_func));
		}
		//TRACE_SOUND(LL_NORMAL, CTX_Default, 0.5f, 1, "Tech-01");
			
		int s = 0;
		for (;;)
		{
			static int i = 0;
			//s += 100;
			
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			//TRACE_MSG(LL_INFO, CTX_Main,  "normal message i=%u from main thread", i);
			//TRACE_MSG(LL_VERBOSE, CTX_Main,  "verbose message i=%u from main thread", i);
			//TRACE_MSG(LL_DEBUG, CTX_Main, "debug message i=%u from main thread", i);
			//TRACE_MSG(LL_WARNING, CTX_Main, "warn message i=%u from render subsys", i);
			//TRACE_MSG(LL_ERROR, CTX_Main, "err message i=%u from render subsys", i);
			TRACE_MSG(LL_SPAM, CTX_Main, "err message i=%u from render subsys", i);

			++i;
			if (i == 20)
				break;
		}

		g_Quit = 1;
		for (std::thread & t : thrs)
			t.join();
	}

	TRACE_DISCONNECT();
	TRACE_DONE();
}

