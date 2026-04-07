#pragma once

#define TODO(msg) do{printf("%s.%d: " msg, __FILE__, __LINE__); exit(99);}while(0);

#define LOG(fmt, ...) do{printf(fmt, __VA_ARGS__);}while(0);
