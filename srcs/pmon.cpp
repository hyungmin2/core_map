#include "pmon.h"
#include "core_map.h"
#include "msr.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <stdint.h>
// #include <sys/mman.h>
// #include <errno.h>


typedef struct LLC_CHA {
  short PMON_UNIT_STATUS;
  short PMON_UNIT_CTL;
  short MSR_PMON_CTR[NUM_CTRS_PER_UNIT];
  short MSR_PMON_CTL[NUM_CTRS_PER_UNIT];
  short MSR_PMON_BOX_FILTER[2];
} LLC_CHA;

LLC_CHA C[NUM_UNITS] = {
  {0x0E07, 0x0E00,
  {0x0E08, 0x0E09, 0x0E0A, 0x0E0B},
  {0x0E01, 0x0E02, 0x0E03, 0x0E04},
  {0x0E05, 0x0E06}},
};


void init_llc_cha_ary() {
  for (int i=0; i<NUM_UNITS-1; i++) {
    C[i+1].PMON_UNIT_STATUS = C[i].PMON_UNIT_STATUS + 0x10;
    C[i+1].PMON_UNIT_CTL    = C[i].PMON_UNIT_CTL    + 0x10;
    for (int j=0; j<NUM_CTRS_PER_UNIT; j++) {
      C[i+1].MSR_PMON_CTR[j] = C[i].MSR_PMON_CTR[j] + 0x10;
      C[i+1].MSR_PMON_CTL[j] = C[i].MSR_PMON_CTL[j] + 0x10;
    }
    C[i+1].MSR_PMON_BOX_FILTER[0] = C[i].MSR_PMON_BOX_FILTER[0] + 0x10;
    C[i+1].MSR_PMON_BOX_FILTER[1] = C[i].MSR_PMON_BOX_FILTER[1] + 0x10;
  }
}


PMON::PMON(int _base_core_id, int _num_cores, int _num_physical_cores) {
  base_core_id = _base_core_id;
  num_cores = _num_cores;
  num_physical_cores = _num_physical_cores;

  msr_h = new MsrHandle(base_core_id);  
  init_llc_cha_ary();
}

PMON::~PMON() {
  delete msr_h;
}



int PMON::check_unique_ppin() {
  uint64 v;
  msr_h->read(0x4E,&v);

  if(v & 0x3 != 0x2) {
    printf("MSR_PPIN_CTL[1:0] != 10b\n");
    return -1;
  }
  
  msr_h->read(0x4F,&ppin);
  printf("PPIN: %llx\n",ppin);
  return 0;
}



// #define U_MSR_PMON_GLOBAL_CTL 0x0700
// U_MSR_PMON_GLOBAL_CTL.frz_all = 1
int PMON::freeze() { 
    uint64 value = (1ULL << 63ULL);
    if (msr_h->write(0x0700, value) == -1) {
      std::cerr << "error at msr_h.write("
                    << std::hex << U_MSR_PMON_GLOBAL_CTL << ", "
                    << std::hex << value << ") in freeze() [" 
                    << std::dec << errno << "]\n";
      return -1;
    }
    return 0;
}
// U_MSR_PMON_GLOBAL_CTL.unfrz_all = 1
int PMON::unfreeze() { 
    uint64 value = (1ULL << 61ULL);
    if (msr_h->write(0x0700, value) == -1) {
      std::cerr << "error at msr_h.write("
                    << std::hex << U_MSR_PMON_GLOBAL_CTL << ", "
                    << std::hex << value << ") in unfreeze() [" 
                    << std::dec << errno << "]\n";
      return -1;
    }
    return 0;
}


 
void PMON::set_pmon_ring_channels()
{
  uint64 event_unit, ret;

	for(int cha_num = 0; cha_num < num_physical_cores; cha_num++) {
    ret = msr_h->write(C[cha_num].PMON_UNIT_CTL, 0x3);
    if(ret != 8) {
        std::cout << "Write(PMON_UNIT_CTL 0x3) error!" << std::endl;
    }        
		// Set Cn_MSR_PMON.VERT_RING_AD_IN_USE.UP
		event_unit = (1ULL << 22ULL) | 0xaa | (1ULL << 17ULL) | (0b00000011 << 8ULL);
		ret = msr_h->write(C[cha_num].MSR_PMON_CTL[0], event_unit);
		if(ret != 8) {
			std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
		}
		// Set Cn_MSR_PMON.VERT_RING_AD_IN_USE.DOWN
		event_unit = (1ULL << 22ULL) | 0xaa | (1ULL << 17ULL) | (0b00001100 << 8ULL);
		ret = msr_h->write(C[cha_num].MSR_PMON_CTL[1], event_unit);
		if(ret != 8) {
			std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
		}
		// Set Cn_MSR_PMON.HORZ_RING_AD_IN_USE.LEFT
		event_unit = (1ULL << 22ULL) | 0xab | (1ULL << 17ULL) | (0b00000011 << 8ULL);
		ret = msr_h->write(C[cha_num].MSR_PMON_CTL[2], event_unit);
		if(ret != 8) {
			std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
		}
		// Set Cn_MSR_PMON.HORZ_RING_AD_IN_USE.RIGHT
		event_unit = (1ULL << 22ULL) | 0xab | (1ULL << 17ULL) | (0b00001100 << 8ULL);
		ret = msr_h->write(C[cha_num].MSR_PMON_CTL[3], event_unit);
		if(ret != 8) {
			std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
		}
	}
}

void PMON::set_llc_pmon()
{
  uint64 event_unit, ret;

  for(int cha_num = 0; cha_num < num_physical_cores; cha_num++) {
    ret = msr_h->write(C[cha_num].PMON_UNIT_CTL, 0x3);
    if(ret != 8) {
        std::cout << "Write(PMON_UNIT_CTL 0x3) error!" << std::endl;
    }

    // Set Cn_MSR_PMON_CTL[0] = TOR_OCCUPANCY.IA_MISS
    event_unit = (1ULL << 22ULL) | 0x36 | (1ULL << 17ULL) | (0b00100001 << 8ULL);
    ret = msr_h->write(C[cha_num].MSR_PMON_CTL[0], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
    }
    // Set Cn_MSR_PMON_CTL[1] = LLC_LOOKUP.ANY                                        <<----!!! find_local_lines use this. LLC_LOOKUP event code = 0x34. Unitmask ANY = 0b00010001 
    event_unit = (1ULL << 22ULL) | 0x34 | (1ULL << 17ULL) | (0b00010001 << 8ULL);
    ret = msr_h->write(C[cha_num].MSR_PMON_CTL[1], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
    }
    // Set Cn_MSR_PMON_CTL[2] = LLC_VICTIMS
    event_unit = (1ULL << 22ULL) | 0x37 | (1ULL << 17ULL) | (0b00101111 << 8ULL);
    ret = msr_h->write(C[cha_num].MSR_PMON_CTL[2], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
    }
    // Set Cn_MSR_PMON_BOX_FILTER0
    event_unit = (0b01100000 << 17ULL); //M and E state
    ret = msr_h->write(C[cha_num].MSR_PMON_BOX_FILTER[0], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
    }
    // Set Cn_MSR_PMON_BOX_FILTER1
    event_unit = (0x259 << 19ULL) | (0x201 << 9ULL) | (1ULL << 5ULL) |
                 (1ULL << 4ULL)   | (0ULL << 3ULL)  | (1ULL << 1ULL) | (1ULL << 0ULL);
    ret = msr_h->write(C[cha_num].MSR_PMON_BOX_FILTER[1], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
    }
  }
} 


std::vector<core_counters> PMON::read_counters() {
  std::vector<core_counters> res;
  for (int i=0; i<num_physical_cores; i++) {
    core_counters counters;    
    msr_h->read(C[i].MSR_PMON_CTR[0], &counters.ctr0);
    msr_h->read(C[i].MSR_PMON_CTR[1], &counters.ctr1);
    msr_h->read(C[i].MSR_PMON_CTR[2], &counters.ctr2);
    msr_h->read(C[i].MSR_PMON_CTR[3], &counters.ctr3);
    res.push_back(counters);
  }
  return res;
}

void PMON::print_busy_path(int core_id,int data_loc_cha_id,uint64 limit) {
  std::vector<core_counters> counters = read_counters();

  std::cout << "busy: " << core_id << " " << data_loc_cha_id <<" ";
  
	for (int i=0; i<num_physical_cores; i++) {
    if(counters[i].ctr0 > limit) {
      std::cout << "C:" << i << " UP " << counters[i].ctr0 << " ";
    }
    else {
      // std::cout << "X:" << i << " UP " << counters[i].ctr0 << " ";
    }
    if(counters[i].ctr1 > limit) {
      std::cout << "C:" << i << " DOWN " << counters[i].ctr1 << " ";
    }
    else {
      // std::cout << "X:" << i << " DOWN " << counters[i].ctr1 << " ";
    }
    if(counters[i].ctr2 > limit) {
      std::cout << "C:" << i << " LEFT " << counters[i].ctr2 << " ";
    }
    else {
      // std::cout << "X:" << i << " LEFT " << counters[i].ctr2 << " ";
    }
    if(counters[i].ctr3 > limit) {
      std::cout << "C:" << i << " RIGHT " << counters[i].ctr3 << " ";
    }
    else {
      // std::cout << "X:" << i << " RIGHT " << counters[i].ctr3 << " ";
    }
  }
  std::cout << std::endl;
}



void PMON::enable_prefetch() {
  for (int i=base_core_id; i<base_core_id+num_cores; i++) {
		MsrHandle socket(i);
		socket.write(0x1a4, 0x0);
	}
}
void PMON::disable_prefetch() {
  for (int i=base_core_id; i<base_core_id+num_cores; i++) {
		MsrHandle socket(i);
		socket.write(0x1a4, 0xF);
	}
}
