#pragma once

#include <cstddef>

extern "C" void* __task_operator_new(size_t);
extern "C" void __task_operator_delete(void*);

#ifdef __cplusplus
extern "C" void __launch_async(void*);
#endif
