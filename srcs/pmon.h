#ifndef __PMON_H__
#define __PMON_H__

#include <stdint.h>
#include <vector>
#include "msr.h"

// #define U_MSR_PMON_GLOBAL_CTL 0x0700

enum ring_channels {CH_UP,CH_DOWN,CH_LEFT,CH_RIGHT};

struct core_counters{
  uint64 ctr0;
  uint64 ctr1;
  uint64 ctr2;
  uint64 ctr3;
};

class PMON{
  public:
    PMON(int _base_core_id, int _num_cores, int _num_physical_cores);
    virtual ~PMON();

    int freeze();
    int unfreeze();

    void set_llc_pmon();
    void set_pmon_ring_channels();

    std::vector<core_counters> read_counters();

    void print_busy_path(int core_id,int data_loc_cha_id, uint64 limit);

    void enable_prefetch();
    void disable_prefetch();

    int check_unique_ppin();

    uint64 get_ppin() {return ppin;}

  private:      
    MsrHandle* msr_h;

    uint64 ppin;

    int base_core_id;
    int num_cores;
    int num_physical_cores;
};


#endif // __PMON_H__