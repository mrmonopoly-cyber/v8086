#pragma once

#include <cstdio>
#include <cstdlib>

#define TODO(msg) do{printf("%s.%d todo:" msg "\n", __FILE__, __LINE__); exit(99);}while(0);

#define LOG(fmt, ...) do{printf(fmt"\n", __VA_ARGS__);}while(0);

#define UNUSED(X) ((void)((X)))
