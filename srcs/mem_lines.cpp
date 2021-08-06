#include <vector>

#include "core_map.h"
#include "mem_lines.h"
#include "tasks.h"
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <fstream>

#include "json.hpp"


Mem_Lines::Mem_Lines(uint _base_core_id, uint _num_cores, uint _num_physical_cores, PMON* _pmon) {
  base_core_id = _base_core_id;
  num_cores = _num_cores;
  num_physical_cores = _num_physical_cores;
  pmon = _pmon;

  memory_bulk_alloc = (char *) malloc(NUM_LINES * LINE_SIZE + 0x1000);
  memory_bulk = (char*) (((uint64_t)memory_bulk_alloc + 0x1000 -1) & ~0xFFFul);
  
  if(mlock(memory_bulk, NUM_LINES * LINE_SIZE) == -1) {
    fprintf(stderr, "Failed to lock page in memory: %s\n", strerror(errno));
    exit(1);
  }

  char * shared_alloc_t = (char *) malloc(NUM_LINES * LINE_SIZE);
  memcpy(memory_bulk,shared_alloc_t,NUM_LINES * LINE_SIZE);
  free(shared_alloc_t);  

  core_map_CHA_to_OS.resize(num_physical_cores);
  for(int i = 0; i < num_physical_cores; i++) core_map_CHA_to_OS[i] = -1;
  core_map_OS_to_CHA.resize(num_cores);
  for(int i = 0; i < num_cores; i++) core_map_OS_to_CHA[i] = -1;
}

Mem_Lines::~Mem_Lines() {
  free(memory_bulk_alloc);  
}

unsigned int get_L2_index(void * addr) {
  return (((uint64_t)addr & 0xFFFF) >> 6);  
}

#define PAGEMAP_LENGTH 8
#define PAGE_SHIFT 12

uint64_t get_page_frame_number_of_address(void *addr) {
   // Open the pagemap file for the current process
   FILE *pagemap = fopen("/proc/self/pagemap", "rb");

   // Seek to the page that the buffer is on
   uint64_t offset = (uint64_t)addr / getpagesize() * PAGEMAP_LENGTH;
   if(fseek(pagemap, (uint64_t)offset, SEEK_SET) != 0) {
      fprintf(stderr, "Failed to seek pagemap to proper location\n");
      exit(1);
   }

   // The page frame number is in bits 0-54 so read the first 7 bytes and clear the 55th bit
   uint64_t page_frame_number = 0;
   size_t reads = fread(&page_frame_number, 1, PAGEMAP_LENGTH-1, pagemap);
   if(reads==0) {
      fprintf(stderr, "Failed to read pagemap\n");
      exit(1);
   }

   page_frame_number &= 0x7FFFFFFFFFFFFF;

   fclose(pagemap);

   return page_frame_number;
}

uint64_t get_physical_address(void * addr) {
   // Get the page frame the buffer is on
   uint64_t page_frame_number = get_page_frame_number_of_address(addr);
   //printf("Page frame: 0x%x\n", page_frame_number);

   // Find the difference from the buffer to the page boundary
   uint64_t distance_from_page_boundary = (unsigned long)addr % getpagesize();

   // Determine how far to seek into memory to find the buffer
   uint64_t offset = (page_frame_number << PAGE_SHIFT) + distance_from_page_boundary;
   
   return offset;
}




#define NUM_RW_TO_LOCATE_LINE 10000000
#define LLC_LOOKUP_THRES_TO_LOCATE_LINE 200000

int Mem_Lines::initialize_local_lines() {
  local_found_count = 0;
  
  uint64_t line = 0;
  while(local_found_count<num_physical_cores*LINES_TO_FIND_PER_CORE) {
    void* acces_ptr = &memory_bulk[line*LINE_SIZE];

    if(get_L2_index( (void*)get_physical_address(acces_ptr) ) == 1) { 
      
      pmon->freeze();
      pmon->set_llc_pmon();      
      
      pmon->unfreeze();             
      read_and_write(base_core_id+0, base_core_id+1, acces_ptr, NUM_RW_TO_LOCATE_LINE);
      pmon->freeze();
      // printf("virtual address %p physical address %lx line %ld\n",&memory_bulk[line*LINE_SIZE],get_physical_address(&memory_bulk[line*LINE_SIZE]),line);

      find_local_lines(line,pmon->read_counters());

      pmon->unfreeze();
    }
    line ++;

    if(line >= NUM_LINES) {
      printf("failed to find enough cache lines for L3 slices. Consider increasing NUM_LINES");
      return -1;
    }
  }
  
  return 0;
}


int Mem_Lines::find_local_lines(int line, std::vector<core_counters> counters) {
  int found = 0;
	local_lines.resize(num_physical_cores);
  
	for (int i=0; i<num_physical_cores; i++) {
    uint64 n_LLC_LOOKUP = counters[i].ctr1;
    if(n_LLC_LOOKUP > LLC_LOOKUP_THRES_TO_LOCATE_LINE) {
	    found += 1;
      if(local_lines[i].size() < LINES_TO_FIND_PER_CORE){
        local_lines[i].push_back(line);
        local_found_count += 1;                
        // std::cout << "local_found " << i << " " << line <<" " <<n_LLC_LOOKUP << "\n";
      }
    }
  }
  if(found != 1) {
    std::cout << "failed to find local line for  " <<line <<" " ;
    for (int i=0; i<num_physical_cores; i++) {      
      uint64_t n_LLC_LOOKUP = counters[i].ctr1;
      std::cout << i <<":" << n_LLC_LOOKUP << " ";
    }
    std::cout << "\n";
    return -1;
  }

  return -0;
}
  

//here, the core id is the CHA id
void* Mem_Lines::get_a_line_for_core(int core_id) {
  return (void*)(&memory_bulk[local_lines[core_id][0]*LINE_SIZE]);
};

std::vector<void*> Mem_Lines::get_lines_for_core(int core_id) {
  std::vector<void*> ptrs;
  for(auto x: local_lines[core_id]) {
    ptrs.push_back(&memory_bulk[x*LINE_SIZE]);
  }

  return ptrs;
}


#define NUM_RW_TO_MAP_CORE 1000000
#define RING_TRAFFIC_THRES_TO_MAP_CORE 10000000


int Mem_Lines::map_os_core_id_to_cha_id() {

  for(int ci = 0; ci < num_cores; ci ++) {
    for(int pi = 0; pi < num_physical_cores; pi ++) {
      pmon->freeze();
      pmon->set_pmon_ring_channels();
      pmon->unfreeze();
            
      read_and_write_spill_to_L3(base_core_id+ci, get_lines_for_core(pi),NUM_RW_TO_MAP_CORE);

      pmon->freeze();
      if(find_core_mapping(ci,pi,pmon->read_counters())) {
        return -1;
      }
      // pmon->print_busy_path(ci,pi,RING_TRAFFIC_THRES_TO_MAP_CORE);
      pmon->unfreeze();
    }
  }

  for(int ci = 0; ci < num_cores; ci ++) {
    if(core_map_OS_to_CHA[ci] == -1) {
      std::cout << "failed to find CHA core ID for OS ID " <<  ci <<std::endl;
      return -1;           
    }
  }

  return 0;
}


int Mem_Lines::find_core_mapping(int ci,int pi,std::vector<core_counters> counters) {
  bool ci_pi_mapped = true;
  for(int pi = 0; pi < num_physical_cores; pi ++) {    
    if( counters[pi].ctr0 > RING_TRAFFIC_THRES_TO_MAP_CORE ||
        counters[pi].ctr1 > RING_TRAFFIC_THRES_TO_MAP_CORE ||
        counters[pi].ctr2 > RING_TRAFFIC_THRES_TO_MAP_CORE ||
        counters[pi].ctr3 > RING_TRAFFIC_THRES_TO_MAP_CORE ) {
      ci_pi_mapped = false;
      break;
    }  
  }

  //all ring channels are idle..
  if(ci_pi_mapped) {
    if(core_map_CHA_to_OS[pi] != -1) {
      std::cout << "Core doubly mapped cha ID " << pi << " for OS ID " << core_map_CHA_to_OS[pi]  << " and " << ci <<std::endl;
      return -1;
    }
    else {
      core_map_CHA_to_OS[pi] = ci;
      core_map_OS_to_CHA[ci] = pi;
    }
  }
  
  return 0;
}



#define NUM_RW_TO_LOCATE_CORE 100000000
#define RING_TRAFFIC_THRES_TO_LOCATE_CORE 1000000


int Mem_Lines::generate_traffic_and_monitor() {

  for(int id_A = 0; id_A < num_physical_cores; id_A ++) {
    int os_id_A = core_map_CHA_to_OS[id_A];
    if(os_id_A == -1) continue;    

    for(int id_B = 0; id_B < num_physical_cores; id_B ++) {
      int os_id_B = core_map_CHA_to_OS[id_B];
      if(os_id_B == -1) continue;

      // printf("A %d:%d B %d:%d \n",id_A,os_id_A,id_B,os_id_B);

      pmon->freeze();
      pmon->set_pmon_ring_channels();
      pmon->unfreeze();

      read_and_write(base_core_id+os_id_A, base_core_id+os_id_B, get_a_line_for_core(id_A),NUM_RW_TO_LOCATE_CORE);
      
      pmon->freeze();
      collect_busy_paths(id_A,id_B,pmon->read_counters(),RING_TRAFFIC_THRES_TO_LOCATE_CORE);
      pmon->print_busy_path(id_A,id_B,RING_TRAFFIC_THRES_TO_LOCATE_CORE);
      pmon->unfreeze();
    }
  }

  return 0;
}



int Mem_Lines::collect_busy_paths(int id_A,int id_B, std::vector<core_counters> counters,uint64 limit) {
  std::vector< std::pair<int,ring_channels> > busy_path;

  for(int pi = 0; pi < num_physical_cores; pi ++) {    
    if( counters[pi].ctr0 > limit ) {
      busy_path.push_back( std::pair<int,ring_channels>(pi,CH_UP) );
    }
    if( counters[pi].ctr1 > limit ) {
      busy_path.push_back( std::pair<int,ring_channels>(pi,CH_DOWN) );
    }
    if( counters[pi].ctr2 > limit ) {
      busy_path.push_back( std::pair<int,ring_channels>(pi,CH_LEFT) );
    }
    if( counters[pi].ctr3 > limit ) {
      busy_path.push_back( std::pair<int,ring_channels>(pi,CH_RIGHT) );
    }  
  }


  busy_paths[std::pair<int,int>(id_A,id_B)] = busy_path;

  return 0;
}


using json = nlohmann::json;

int Mem_Lines::dump_busy_paths(std::string filename)  {
  json j;
  
  j["ppin"] = std::to_string(pmon->get_ppin());

  j["CHA_to_os"] = json::array();
  for(int i = 0; i < core_map_CHA_to_OS.size(); i ++ ) {
    j["CHA_to_os"].push_back(core_map_CHA_to_OS[i]);
  }
   

  j["busy_paths"] = json::array();

  for(const auto& kv: busy_paths)  {
    int id_A = kv.first.first;
    int id_B = kv.first.second;
    auto path = kv.second;

    json jp;
    jp["id_A"] = id_A;
    jp["id_B"] = id_B;

    for(const auto& node: path)  {
      int node_id = node.first;
      ring_channels channel = node.second;
      jp["path"].push_back( { {"node_id",node_id}, {"channel",channel} } );
    }

    j["busy_paths"] .push_back(jp);

  }
   
  std::ofstream o(filename);
  o << std::setw(2) << j << std::endl;
}

