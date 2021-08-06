#ifndef __CPUID_TASKS_H__
#define __CPUID_TASKS_H__

#include <cpuid.h>

int check_brand_string();
long get_processor_count();
int check_processor_family_model_id();

#endif // __CPUID_TASKS_H__