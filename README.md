# Core Map

Map individual processor cores of the Intel Xeon CPUs on the mesh interconnect .

* Currently works well only for socket 0 (Need some parameter tuning for socket 1)
* Need sudo privilege
* Supports Scalable Gen 1 (Skylake) and Gen2 (Cascade lake)
* Gen 3 (Ice lake) support will be added soon


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
* Need to know the number of processor cores (#CPU) and the number of LLC slices (#LLC)
* NLLC = total L3 cache size in KB (can be identified using 'cat /proc/cpuinfo')  / 1375
  (e.g., cache size      : 33792 KB -> 24 LLCs)
  In many cases, #CPU == #LLC, but some CPUs have LLC-only tiles and #LLC can be larger than #CPU


```cosole
sudo modprobe msr    #need only once
sudo ./core_map -b 0 -c <#LLC> -p <#LLC>
```

* After obtaining the monitoring file, perform ILP solving to map the processor core locations

```console
python3 layout_ilp_convert.py <busy_path.<PPIN>.json file>
```

