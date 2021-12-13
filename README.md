# Core Map

Map the locations of the processor core tiles of the Intel Xeon CPUs on the mesh interconnect .

For more details about the mechanism, please refer to our DATE 2022 paper, "Know Your Neighbor: Physically Locating Xeon Processor Cores on the Core Tile Grid"

* **Need sudo privilege**
* Supports Scalable Gen 1 (Skylake), Gen2 (Cascade lake) and Gen3 (Ice lake)


## Install CVXPY ILP solver interface
* python 3.7+ 
```
pip install cvxpy
```
* if installation fails due to missing Python.h,  try ``sudo apt install libpython3.7-dev``

## install Coin-OR solver
* https://github.com/coin-or/CyLP/issues/47
```
sudo apt-get install -y git pkg-config coinor-libcbc-dev coinor-libosi-dev coinor-libcoinutils-dev coinor-libcgl-dev
export COIN_INSTALL_DIR=/usr/
pip3 install cylp
```

## How to use
* Need to know the number of processor cores (`#CPU`) and the number of LLC slices (`#LLC`)
* `#CPU` is the number of physical processor cores per socket, excluding the multithreaded (hyperthreaded) cores. 
* `#LLC` = total L3 cache size in KB (can be identified using 'cat /proc/cpuinfo')  / 1408
  * (e.g., cache size      : 33792 KB -> 24 LLCs)
  * In many cases, #CPU == #LLC, but some CPUs have LLC-only tiles and #LLC can be larger than #CPU
* For the Ice lake models, `#LLC` = total L3 cache size in KB / 1536
* `#BASE_ID` is the first core ID of the socket. 
  * For example, if the system is a two-socket system where each CPU chip has 24 cores, `-b 0` maps the CPU on socket 0 and '-b 24' maps the CPU on socket 1.

```console
sudo modprobe msr    #need only once
sudo ./core_map -c <#CPU> -p <#LLC> -b <#BASE_ID> 
```

* The above command will generate a tile-to-tile traffic monitoring file <busy_path.<PPIN>.json file>
* After obtaining the monitoring file, perform ILP solving to map the processor core locations

```console
python3 layout_ilp_convert.py <busy_path.<PPIN>.json file>
```

* If the above layout generation python program fails, try regnerating the the busypath file using the core_map program again.
  
