#include <string>
#include <cstdio>
#include "history.h"

int main ()
{
	History<std::string> h(4);

	h.insert("a");
	h.insert("b");
	h.insert("c");
	h.insert("d");
	h.dump();
	h.insert("a");
	h.insert("b");
	h.insert("c");
	h.insert("e");
	h.dump();
	h.insert("a");
	h.insert("b");
	h.insert("f");
	h.insert("z");
	h.dump();
}
