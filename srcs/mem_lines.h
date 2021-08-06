#ifndef __MEM_LINES_H_
#define __MEM_LINES_H_

#define NUM_LINES  (1024*1024*16)
#define LINES_TO_FIND_PER_CORE   22


#include <stdint.h>
#include <vector>
#include <map>
#include "pmon.h"

class Mem_Lines{
  public:
    Mem_Lines(uint _base_core_id, uint _num_cores, uint _num_physical_cores, PMON* _pmon);

    virtual ~Mem_Lines();

    int initialize_local_lines();

    int map_os_core_id_to_cha_id();

    int generate_traffic_and_monitor();

    //here, the core id is the CHA id
    void* get_a_line_for_core(int core_id);
    std::vector<void*> get_lines_for_core(int core_id);
    int dump_busy_paths(std::string filename);
    
    std::vector<int> core_map_CHA_to_OS;
    std::vector<int> core_map_OS_to_CHA;

  private:      
    char * memory_bulk;
    char * memory_bulk_alloc;

    int base_core_id;
    int num_cores;
    int num_physical_cores;

    PMON* pmon;

    std::vector<std::vector<int>> local_lines;
    int local_found_count;

    int find_local_lines(int line, std::vector<core_counters> counters);
    int find_core_mapping(int ci,int pi,std::vector<core_counters> counters);    
    int collect_busy_paths(int id_A,int id_B, std::vector<core_counters> counters,uint64 limit);


    std::map<std::pair<int,int>, std::vector<std::pair<int,ring_channels> > > busy_paths;    
};


#endif // __MEM_LINES_H_