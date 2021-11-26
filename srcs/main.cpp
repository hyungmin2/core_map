#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "msr.h"
#include "pmon.h"
#include "mem_lines.h"
#include "cpuid_tasks.h"

void print_usage(const char * progname) {
  std::cout << "usage: " << progname 
				<< " [-c num_cores] [-b base_cod_id] [-h]\n";
}

int main(int argc, char *argv[])
{
  int num_cores = 16;
  int base_core_id = 0;
  int num_physical_cores = -1;
  int opt;

  while ((opt = getopt(argc, argv, "hc:b:p:")) != -1) {
   switch(opt) {
     case 'c':
       num_cores = atoi(optarg);
       break;
     case 'b':
       base_core_id = atoi(optarg);
       break;
     case 'p':
       num_physical_cores = atoi(optarg);
       break;
     case 'h':
     default:
       print_usage(argv[0]);
       return -1;
   }
  }
  if(num_physical_cores==-1) num_physical_cores= num_cores;

  get_processor_count();
  check_brand_string();  
  int processor_model = check_processor_family_model_id();
  if(processor_model < 0) exit(-1);


  printf("num_cores %d\n",num_cores);
  printf("base_core_id %d\n",base_core_id);
  printf("num_physical_cores %d\n",num_physical_cores);
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(base_core_id, &cpuset);
  pthread_t current_thread = pthread_self();    
  pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);

  PMON * pmon = new PMON(processor_model,base_core_id,num_cores,num_physical_cores);  
  pmon->check_unique_ppin();

  pmon->disable_prefetch();

  Mem_Lines * mem_lines = new Mem_Lines(base_core_id,num_cores,num_physical_cores,pmon);
  
  mem_lines->initialize_local_lines();
  
  mem_lines->map_os_core_id_to_cha_id();

  mem_lines->generate_traffic_and_monitor();
  
  pmon->enable_prefetch();

  std::stringstream ppin_str;
  ppin_str << std::hex << pmon->get_ppin();

  mem_lines->dump_busy_paths("busy_path." + ppin_str.str() + ".json");

  delete mem_lines;  
  delete pmon;

  return 0;
}

