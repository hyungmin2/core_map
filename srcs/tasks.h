#ifndef __TASKS_H__
#define __TASKS_H__

#include "types.h"
#include <pthread.h>
#include <vector>

int read_and_write(int core_a,int core_b, void* access_ptr, uint64 repeat);
int read_and_write_spill_to_L3(int core_id, std::vector<void*> ptrs, uint64_t repeat);

#endif // __TASKS_H__