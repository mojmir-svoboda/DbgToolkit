#pragma once

#define TOKENCONCAT(x, y) x ## y
#define TOKENCONCAT2(x, y) TOKENCONCAT(x, y)
#define UNIQUE(name) TOKENCONCAT2(name, __LINE__)
