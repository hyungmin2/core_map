#include "tasks.h"
#include <stdint.h>

struct job_info{
    void* access_ptr;
    uint64_t repeat;
};


void *rw_job(void *ptr)
{
    struct job_info* ji = (struct job_info *)ptr;
    volatile uint8_t *p = (uint8_t *)ji->access_ptr;
    
    uint64_t i = 0;
    while (i < ji->repeat) {
        *p = *p + 1;
        i  =  i + 1;
    }
    
    return NULL;
}

void *ro_job(void *ptr)
{
    struct job_info* ji = (struct job_info *)ptr;
    volatile uint8_t *p = (uint8_t *)ji->access_ptr;
    
    uint64_t i = 0;
    uint64_t res = *p;
    while (i < ji->repeat) {
        res = *p;
        i  =  i + 1;
    }
    
    return (void*)res;
}

//here the core id is the OS core id
int read_and_write(int core_a,int core_b, void* access_ptr, uint64 repeat)
{
    cpu_set_t cpuset1, cpuset2;
    pthread_t thread1, thread2;
    
    CPU_ZERO(&cpuset1); CPU_SET(core_a, &cpuset1);
    CPU_ZERO(&cpuset2); CPU_SET(core_b, &cpuset2);
   
    struct job_info ji;
    ji.access_ptr = access_ptr;
    ji.repeat = repeat;
    
    pthread_create(&thread1, NULL, rw_job, &ji);
    pthread_create(&thread2, NULL, ro_job, &ji);
    pthread_setaffinity_np(thread1, sizeof(cpu_set_t), &cpuset1);
    pthread_setaffinity_np(thread2, sizeof(cpu_set_t), &cpuset2);
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    return 0;
}



int read_and_write_spill_to_L3(int core_id, std::vector<void*> ptrs, uint64_t repeat)
{
    cpu_set_t cpuset;
    
    CPU_ZERO(&cpuset); CPU_SET(core_id, &cpuset);
    
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    
    uint64_t i = 0;
    uint64_t res;
    while (i < repeat) {
      for(int fi = 0; fi < ptrs.size(); fi ++)  {
        volatile uint8_t *p = (uint8_t *)(ptrs[fi]);
        *p = *p + 1;
        res = *p;
      }
      i  =  i + 1;
    }
    
    return res;
}
