#pragma once

#define TOKEN_CONCAT(x, y) x ## y
#define TOKEN_CONCAT2(x, y) TOKEN_CONCAT(x, y)
#define UNIQUE(name) TOKEN_CONCAT2(name, __LINE__)
