#include "cpuid_tasks.h"
#include "core_map.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>


int check_brand_string() {
  uint32_t brand[12];

  if (!__get_cpuid_max(0x80000004, NULL)) {
      fprintf(stderr, "Brand string not implemented.");
      return -1;
  }

  __get_cpuid(0x80000002, brand+0x0, brand+0x1, brand+0x2, brand+0x3);
  __get_cpuid(0x80000003, brand+0x4, brand+0x5, brand+0x6, brand+0x7);
  __get_cpuid(0x80000004, brand+0x8, brand+0x9, brand+0xa, brand+0xb);
  printf("Brand: %s\n", (char*)brand);  

  return 0;
}

long get_processor_count() {
  long num_processor_configured = sysconf (_SC_NPROCESSORS_CONF);
  long num_processor_online= sysconf (_SC_NPROCESSORS_ONLN);

  return num_processor_online;
}


int check_processor_family_model_id() {
  //CPUID EAX:1 
  uint32_t eax,ebx,ecx,edx;
  __cpuid(1,eax,ebx,ecx,edx);
  
  uint32_t family_id_e = (eax >> 20) & 0xFF;
  uint32_t family_id = (eax >> 8) & 0xF;

  uint32_t model_id_e = (eax >> 16) & 0xF;
  uint32_t model_id = (eax >> 4) & 0xF;

  if(family_id == 6 || family_id==15) {
      model_id = (model_id_e << 4) + model_id;
  }
  if( family_id==15) {
    family_id = family_id + family_id_e;
  }

  if(family_id != 6 || (model_id != CORE_MODEL_SKL &&  model_id != CORE_MODEL_ICL)) {
    printf("core_map is tested on Intel Xeon CPUs with family ID 6 and model ID %d or %d only! current family_id:%d model_id:%d\n",CORE_MODEL_SKL,CORE_MODEL_ICL,family_id,model_id);
    return -1;
  }
  else {
    return model_id;
  }
}