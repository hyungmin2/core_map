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

  check_brand_string();  
  if(check_processor_family_model_id() != 0) exit(-1);

  
  PMON * pmon = new PMON(base_core_id,num_cores,num_physical_cores);  
  pmon->check_unique_ppin();

  pmon->disable_prefetch();

  Mem_Lines * mem_lines = new Mem_Lines(base_core_id,num_cores,num_physical_cores,pmon);
  
  mem_lines->initialize_local_lines();
  
  mem_lines->map_os_core_id_to_cha_id();

  mem_lines->generate_traffic_and_monitor();
  
  pmon->enable_prefetch();

  mem_lines->dump_busy_paths("busy_path." + std::to_string(pmon->get_ppin()) + ".json");

  delete mem_lines;  
  delete pmon;

  return 0;
}

