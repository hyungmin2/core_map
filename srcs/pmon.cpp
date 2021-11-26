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
} LLC_CHA_SKL;


typedef struct LLC_CHA_ICL {
  short PMON_UNIT_STATUS;
  short PMON_UNIT_CTL;
  short MSR_PMON_CTR[NUM_CTRS_PER_UNIT];
  short MSR_PMON_CTL[NUM_CTRS_PER_UNIT];
  short MSR_PMON_BOX_FILTER;
} LLC_CHA_ICL;

LLC_CHA_SKL C_SKL[NUM_UNITS_SKL] = {
  {0x0E07, 0x0E00,
  {0x0E08, 0x0E09, 0x0E0A, 0x0E0B},
  {0x0E01, 0x0E02, 0x0E03, 0x0E04},
  {0x0E05, 0x0E06}},
};

void init_llc_cha_ary_skl() {
  for (int i=0; i<NUM_UNITS_SKL-1; i++) {
    C_SKL[i+1].PMON_UNIT_STATUS = C_SKL[i].PMON_UNIT_STATUS + 0x10;
    C_SKL[i+1].PMON_UNIT_CTL    = C_SKL[i].PMON_UNIT_CTL    + 0x10;
    for (int j=0; j<NUM_CTRS_PER_UNIT; j++) {
      C_SKL[i+1].MSR_PMON_CTR[j] = C_SKL[i].MSR_PMON_CTR[j] + 0x10;
      C_SKL[i+1].MSR_PMON_CTL[j] = C_SKL[i].MSR_PMON_CTL[j] + 0x10;
    }
    C_SKL[i+1].MSR_PMON_BOX_FILTER[0] = C_SKL[i].MSR_PMON_BOX_FILTER[0] + 0x10;
    C_SKL[i+1].MSR_PMON_BOX_FILTER[1] = C_SKL[i].MSR_PMON_BOX_FILTER[1] + 0x10;
  }
}

LLC_CHA_ICL C_ICL[NUM_UNITS_ICL];

void init_llc_cha_ary_icl() {
  C_ICL[0].PMON_UNIT_STATUS = 0x0E07;
  C_ICL[0].PMON_UNIT_CTL = 0x0E00;
  C_ICL[0].MSR_PMON_CTR[0] = 0x0E0B;
  C_ICL[0].MSR_PMON_CTR[1] = 0x0E0A;
  C_ICL[0].MSR_PMON_CTR[2] = 0x0E09;
  C_ICL[0].MSR_PMON_CTR[3] = 0x0E08;
  C_ICL[0].MSR_PMON_CTL[0] = 0x0E04;
  C_ICL[0].MSR_PMON_CTL[1] = 0x0E03;
  C_ICL[0].MSR_PMON_CTL[2] = 0x0E02;
  C_ICL[0].MSR_PMON_CTL[3] = 0x0E01;
  C_ICL[0].MSR_PMON_BOX_FILTER = 0x0E05;
  
  for (int i=0; i<18-1; i++) {
    C_ICL[i+1].PMON_UNIT_STATUS = C_ICL[i].PMON_UNIT_STATUS + 0xE;
    C_ICL[i+1].PMON_UNIT_CTL    = C_ICL[i].PMON_UNIT_CTL    + 0xE;
    for (int j=0; j<NUM_CTRS_PER_UNIT; j++) {
      C_ICL[i+1].MSR_PMON_CTR[j] = C_ICL[i].MSR_PMON_CTR[j] + 0xE;
      C_ICL[i+1].MSR_PMON_CTL[j] = C_ICL[i].MSR_PMON_CTL[j] + 0xE;
    }
    C_ICL[i+1].MSR_PMON_BOX_FILTER = C_ICL[i].MSR_PMON_BOX_FILTER + 0xE;
  }

  C_ICL[18].PMON_UNIT_STATUS = 0x0F11;
  C_ICL[18].PMON_UNIT_CTL = 0x0F0A;
  C_ICL[18].MSR_PMON_CTR[0] = 0x0F15;
  C_ICL[18].MSR_PMON_CTR[1] = 0x0F14;
  C_ICL[18].MSR_PMON_CTR[2] = 0x0F13;
  C_ICL[18].MSR_PMON_CTR[3] = 0x0F12;
  C_ICL[18].MSR_PMON_CTL[0] = 0x0F0E;
  C_ICL[18].MSR_PMON_CTL[1] = 0x0F0D;
  C_ICL[18].MSR_PMON_CTL[2] = 0x0F0C;
  C_ICL[18].MSR_PMON_CTL[3] = 0x0F0B;
  C_ICL[18].MSR_PMON_BOX_FILTER = 0x0F0F;
  
  for (int i=18; i<34-1; i++) {
    C_ICL[i+1].PMON_UNIT_STATUS = C_ICL[i].PMON_UNIT_STATUS + 0xE;
    C_ICL[i+1].PMON_UNIT_CTL    = C_ICL[i].PMON_UNIT_CTL    + 0xE;
    for (int j=0; j<NUM_CTRS_PER_UNIT; j++) {
      C_ICL[i+1].MSR_PMON_CTR[j] = C_ICL[i].MSR_PMON_CTR[j] + 0xE;
      C_ICL[i+1].MSR_PMON_CTL[j] = C_ICL[i].MSR_PMON_CTL[j] + 0xE;
    }
    C_ICL[i+1].MSR_PMON_BOX_FILTER = C_ICL[i].MSR_PMON_BOX_FILTER + 0xE;
  }



  C_ICL[34].PMON_UNIT_STATUS = 0x0B67;
  C_ICL[34].PMON_UNIT_CTL =    0x0B60;
  C_ICL[34].MSR_PMON_CTR[0] =  0x0B6B;
  C_ICL[34].MSR_PMON_CTR[1] =  0x0B6A;
  C_ICL[34].MSR_PMON_CTR[2] =  0x0B69;
  C_ICL[34].MSR_PMON_CTR[3] =  0x0B68;
  C_ICL[34].MSR_PMON_CTL[0] =  0x0B64;
  C_ICL[34].MSR_PMON_CTL[1] =  0x0B63;
  C_ICL[34].MSR_PMON_CTL[2] =  0x0B62;
  C_ICL[34].MSR_PMON_CTL[3] =  0x0B61;
  C_ICL[34].MSR_PMON_BOX_FILTER = 0x0B65;
  
  for (int i=34; i<39-1; i++) {
    C_ICL[i+1].PMON_UNIT_STATUS = C_ICL[i].PMON_UNIT_STATUS + 0xE;
    C_ICL[i+1].PMON_UNIT_CTL    = C_ICL[i].PMON_UNIT_CTL    + 0xE;
    for (int j=0; j<NUM_CTRS_PER_UNIT; j++) {
      C_ICL[i+1].MSR_PMON_CTR[j] = C_ICL[i].MSR_PMON_CTR[j] + 0xE;
      C_ICL[i+1].MSR_PMON_CTL[j] = C_ICL[i].MSR_PMON_CTL[j] + 0xE;
    }
    C_ICL[i+1].MSR_PMON_BOX_FILTER = C_ICL[i].MSR_PMON_BOX_FILTER + 0xE;
  }
}



PMON::PMON(int _processor_model, int _base_core_id, int _num_cores, int _num_physical_cores) {
  processor_model = _processor_model;
  base_core_id = _base_core_id;
  num_cores = _num_cores;
  num_physical_cores = _num_physical_cores;

  msr_h = new MsrHandle(base_core_id);  
  
  if(processor_model == CORE_MODEL_SKL)
    init_llc_cha_ary_skl();
  else if(processor_model == CORE_MODEL_ICL)
    init_llc_cha_ary_icl();
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


 
void PMON::set_pmon_ring_channels() {
  if(processor_model == CORE_MODEL_SKL) 
    set_pmon_ring_channels_skl();
  else if(processor_model == CORE_MODEL_ICL)  
    set_pmon_ring_channels_icl();
}

void PMON::set_pmon_ring_channels_skl()
{
  uint64 event_unit, ret;

	for(int cha_num = 0; cha_num < num_physical_cores; cha_num++) {
    ret = msr_h->write(C_SKL[cha_num].PMON_UNIT_CTL, 0x3);
    if(ret != 8) {
        std::cout << "Write(PMON_UNIT_CTL 0x3) error!" << std::endl;
    }        
		// Set Cn_MSR_PMON.VERT_RING_AD_IN_USE.UP
		event_unit = (1ULL << 22ULL) | 0xaa | (1ULL << 17ULL) | (0b00000011 << 8ULL);
		ret = msr_h->write(C_SKL[cha_num].MSR_PMON_CTL[0], event_unit);
		if(ret != 8) {
			std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
		}
		// Set Cn_MSR_PMON.VERT_RING_AD_IN_USE.DOWN
		event_unit = (1ULL << 22ULL) | 0xaa | (1ULL << 17ULL) | (0b00001100 << 8ULL);
		ret = msr_h->write(C_SKL[cha_num].MSR_PMON_CTL[1], event_unit);
		if(ret != 8) {
			std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
		}
		// Set Cn_MSR_PMON.HORZ_RING_AD_IN_USE.LEFT
		event_unit = (1ULL << 22ULL) | 0xab | (1ULL << 17ULL) | (0b00000011 << 8ULL);
		ret = msr_h->write(C_SKL[cha_num].MSR_PMON_CTL[2], event_unit);
		if(ret != 8) {
			std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
		}
		// Set Cn_MSR_PMON.HORZ_RING_AD_IN_USE.RIGHT
		event_unit = (1ULL << 22ULL) | 0xab | (1ULL << 17ULL) | (0b00001100 << 8ULL);
		ret = msr_h->write(C_SKL[cha_num].MSR_PMON_CTL[3], event_unit);
		if(ret != 8) {
			std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
		}
	}
}


void PMON::set_pmon_ring_channels_icl()
{
  uint64 event_unit, ret;

	for(int cha_num = 0; cha_num < num_physical_cores; cha_num++) {
    ret = msr_h->write(C_ICL[cha_num].PMON_UNIT_CTL, 0x3);
    if(ret != 8) {
        std::cout << "Write(PMON_UNIT_CTL 0x3) error!" << std::endl;
    }        
		// Set Cn_MSR_PMON.VERT_RING_AD_IN_USE.UP
		event_unit = (1ULL << 22ULL) | 0xb2 | (1ULL << 17ULL) | (0b00000011 << 8ULL);
		ret = msr_h->write(C_ICL[cha_num].MSR_PMON_CTL[0], event_unit);
		if(ret != 8) {
			std::cout << "Write(en & umask & ev_sel & rst) 1 error!" << std::endl;
		}
		// Set Cn_MSR_PMON.VERT_RING_AD_IN_USE.DOWN
		event_unit = (1ULL << 22ULL) | 0xb2 | (1ULL << 17ULL) | (0b00001100 << 8ULL);
		ret = msr_h->write(C_ICL[cha_num].MSR_PMON_CTL[1], event_unit);
		if(ret != 8) {
			std::cout << "Write(en & umask & ev_sel & rst) 2 error!" << std::endl;
		}
		// Set Cn_MSR_PMON.HORZ_RING_AD_IN_USE.LEFT
		event_unit = (1ULL << 22ULL) | 0xb8 | (1ULL << 17ULL) | (0b00000011 << 8ULL);
		ret = msr_h->write(C_ICL[cha_num].MSR_PMON_CTL[2], event_unit);
		if(ret != 8) {
			std::cout << "Write(en & umask & ev_sel & rst) 3 error!" << std::endl;
		}
		// Set Cn_MSR_PMON.HORZ_RING_AD_IN_USE.RIGHT
		event_unit = (1ULL << 22ULL) | 0xb8 | (1ULL << 17ULL) | (0b00001100 << 8ULL);
		ret = msr_h->write(C_ICL[cha_num].MSR_PMON_CTL[3], event_unit);
		if(ret != 8) {
			std::cout << "Write(en & umask & ev_sel & rst) 4 error!" << std::endl;
		}
	}
}

void PMON::set_llc_pmon() {
  if(processor_model == CORE_MODEL_SKL) 
    set_llc_pmon_skl();
  else if(processor_model == CORE_MODEL_ICL) 
    set_llc_pmon_icl();
}

void PMON::set_llc_pmon_skl()
{
  uint64 event_unit, ret;

  for(int cha_num = 0; cha_num < num_physical_cores; cha_num++) {
    ret = msr_h->write(C_SKL[cha_num].PMON_UNIT_CTL, 0x3);
    if(ret != 8) {
        std::cout << "Write(PMON_UNIT_CTL 0x3) error!" << std::endl;
    }

    // Set Cn_MSR_PMON_CTL[0] = TOR_OCCUPANCY.IA_MISS
    event_unit = (1ULL << 22ULL) | 0x36 | (1ULL << 17ULL) | (0b00100001 << 8ULL);
    ret = msr_h->write(C_SKL[cha_num].MSR_PMON_CTL[0], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
    }
    // Set Cn_MSR_PMON_CTL[1] = LLC_LOOKUP.ANY                                        <<----!!! find_local_lines use this. LLC_LOOKUP event code = 0x34. Unitmask ANY = 0b00010001 
    event_unit = (1ULL << 22ULL) | 0x34 | (1ULL << 17ULL) | (0b00010001 << 8ULL);
    ret = msr_h->write(C_SKL[cha_num].MSR_PMON_CTL[1], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
    }
    // Set Cn_MSR_PMON_CTL[2] = LLC_VICTIMS
    event_unit = (1ULL << 22ULL) | 0x37 | (1ULL << 17ULL) | (0b00101111 << 8ULL);
    ret = msr_h->write(C_SKL[cha_num].MSR_PMON_CTL[2], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
    }
    // Set Cn_MSR_PMON_BOX_FILTER0
    event_unit = (0b01100000 << 17ULL); //M and E state
    ret = msr_h->write(C_SKL[cha_num].MSR_PMON_BOX_FILTER[0], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
    }
    // Set Cn_MSR_PMON_BOX_FILTER1
    event_unit = (0x259 << 19ULL) | (0x201 << 9ULL) | (1ULL << 5ULL) |
                 (1ULL << 4ULL)   | (0ULL << 3ULL)  | (1ULL << 1ULL) | (1ULL << 0ULL);
    ret = msr_h->write(C_SKL[cha_num].MSR_PMON_BOX_FILTER[1], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) error!" << std::endl;
    }
  }
} 

void PMON::set_llc_pmon_icl()
{
  uint64 event_unit, ret;

  for(int cha_num = 0; cha_num < num_physical_cores; cha_num++) {
    ret = msr_h->write(C_ICL[cha_num].PMON_UNIT_CTL, 0x3);
    if(ret != 8) {
        std::cout << "Write(PMON_UNIT_CTL 0x3) error!" << std::endl;
    }

    // Set Cn_MSR_PMON_CTL[0] = TOR_OCCUPANCY.IA_MISS
    event_unit = (1ULL << 22ULL) | 0x36 | (1ULL << 17ULL) | (0b00100001 << 8ULL);
    ret = msr_h->write(C_ICL[cha_num].MSR_PMON_CTL[0], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) 1 error!" << std::endl;
    }
    // Set Cn_MSR_PMON_CTL[1] = LLC_LOOKUP.ANY                                        <<----!!! find_local_lines use this. LLC_LOOKUP event code = 0x34. Unitmask ANY = 0b00010001 
    // event_unit = (1ULL << 22ULL) | 0x34 | (1ULL << 17ULL) | (0b00010001 << 8ULL);
    event_unit = (1ULL << 22ULL) | 0x34ULL | (1ULL << 17ULL)  |  (0x1FFFULL << 32ULL) | (0xFFULL << 8ULL);
    ret = msr_h->write(C_ICL[cha_num].MSR_PMON_CTL[1], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) 2 error!" << std::endl;
    }
      // uint64 r;
      // msr_h->read(C_ICL[cha_num].MSR_PMON_CTL[1], &r);
      // std::cout << "read %llx" << std::hex << r<< std::dec<< std::endl;
     
    // Set Cn_MSR_PMON_CTL[2] = LLC_VICTIMS
    event_unit = (1ULL << 22ULL) | 0x37 | (1ULL << 17ULL) | (0b00101111 << 8ULL);
    ret = msr_h->write(C_ICL[cha_num].MSR_PMON_CTL[2], event_unit);
    if(ret != 8) {
      std::cout << "Write(en & umask & ev_sel & rst) 3 error!" << std::endl;
    }

    // Set Cn_MSR_PMON_BOX_FILTER   
    // // msr_h->read(C_ICL[cha_num].MSR_PMON_BOX_FILTER, &event_unit);     
    // event_unit |= (0b100000ULL << 32ULL) ; //ANY_F, M and E state
    // // event_unit = (0b100000ULL << 32ULL) | (0b01100000 << 8ULL); //ANY_F, M and E state
    // // event_unit = 0;
    
    // ret = msr_h->write(C_ICL[cha_num].MSR_PMON_BOX_FILTER, event_unit);
    // if(ret != 8) {
    //   std::cout << "Write(en & umask & ev_sel & rst) 24 error! " << cha_num<< " " << std::hex << C_ICL[cha_num].MSR_PMON_BOX_FILTER  << std::dec<< std::endl;
    // }
   
  }
} 


std::vector<core_counters> PMON::read_counters() {
  std::vector<core_counters> res;
  for (int i=0; i<num_physical_cores; i++) {
    core_counters counters;    

    if(processor_model == CORE_MODEL_SKL) {
      msr_h->read(C_SKL[i].MSR_PMON_CTR[0], &counters.ctr0);
      msr_h->read(C_SKL[i].MSR_PMON_CTR[1], &counters.ctr1);
      msr_h->read(C_SKL[i].MSR_PMON_CTR[2], &counters.ctr2);
      msr_h->read(C_SKL[i].MSR_PMON_CTR[3], &counters.ctr3);
    }
    else if(processor_model == CORE_MODEL_ICL) {
      msr_h->read(C_ICL[i].MSR_PMON_CTR[0], &counters.ctr0);
      msr_h->read(C_ICL[i].MSR_PMON_CTR[1], &counters.ctr1);
      msr_h->read(C_ICL[i].MSR_PMON_CTR[2], &counters.ctr2);
      msr_h->read(C_ICL[i].MSR_PMON_CTR[3], &counters.ctr3);
    }


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
