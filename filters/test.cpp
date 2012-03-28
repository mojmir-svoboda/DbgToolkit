#include <cstdio>
#include <string>
#include <boost/tokenizer.hpp>
#include "file_filter.hpp"


std::string s[] = { "c:/devel/aa.cpp:21", "c:/devel/aa.cpp/23", "c:/devel/main.cpp", "c:/devel/bb.cpp" };

int main ()
{
	file_filter ff;
	for (size_t i = 0; i < sizeof(s)/sizeof(*s); ++i)
	{
		//printf("iteration %i\n", i);
		ff.append(s[i]);
	}

	printf("*************\n");
	{
		std::string s("c:/devel/aa.cpp");
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());
	}

	{
		std::string s("c:/devel/aa.cpp:21");
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());
		ff.exclude_off(s);
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());

		ff.append(s);
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());
	}
	{
		std::string s("c:/devel/aa.cpp/27");
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());
	}
	{
		std::string s("c:/devel/bb.cpp/27");
		printf("\nexcluded=%u %s\n", ff.is_excluded(s), s.c_str());
	}
	{
		std::string s("c:/devel/bb.cpp");
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());
	}
	{
		std::string s("c:/jewel/bb.cpp");
		printf("excluded=%u %s\n", ff.is_excluded(s), s.c_str());
	}

	std::string out;
	ff.export_filter(out);
	printf("exported: |%s|\n", out.c_str());
}
