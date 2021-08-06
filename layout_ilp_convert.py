import json
import sys
import queue
import cvxpy as cp
import gurobipy

print("Processing:",sys.argv[1])
with open(sys.argv[1], "r") as busy_path_file:
  path_info = json.load(busy_path_file)

busy_paths = path_info["busy_paths"]
CHA_to_os = path_info["CHA_to_os"]

deprecated = [i for (i,x) in enumerate(CHA_to_os) if x == -1]
# print(deprecated)

num_cores = len(CHA_to_os) - len(deprecated)
print("num_cores:",num_cores)


constraints = []

N_VERT=8
N_HORZ=10

vert_index = cp.Variable((num_cores+len(deprecated)), integer=True)
horz_index = cp.Variable((num_cores+len(deprecated)), integer=True)

constraints.append(  vert_index >= 0 )
constraints.append(  horz_index >= 0 )
constraints.append(  vert_index <= N_VERT -1)
constraints.append(  horz_index <= N_HORZ -1)

vert_pos = cp.Variable((num_cores+len(deprecated),N_VERT), boolean=True)
horz_pos = cp.Variable((num_cores+len(deprecated),N_HORZ), boolean=True)

constraints.append(  1 == cp.sum(vert_pos,axis=1) )
constraints.append(  1 == cp.sum(horz_pos,axis=1) )

connect_connst_v = 0*vert_pos[:,0]
for i in range(1,N_VERT):
  connect_connst_v += i*vert_pos[:,i]

connect_connst_h = 0*horz_pos[:,0]
for i in range(1,N_HORZ):
  connect_connst_h += i*horz_pos[:,i]

constraints.append(vert_index == connect_connst_v)
constraints.append(horz_index == connect_connst_h)

vert_group_indicator = cp.Variable((N_VERT), boolean=True)
horz_group_indicator = cp.Variable((N_HORZ), boolean=True)

constraints.append(  vert_group_indicator <= cp.sum(vert_pos,axis=0) )
constraints.append(  cp.sum(vert_pos,axis=0) <= 100*vert_group_indicator )

constraints.append(  horz_group_indicator <= cp.sum(horz_pos,axis=0) )
constraints.append(  cp.sum(horz_pos,axis=0) <= 100*horz_group_indicator )



for busy_path in busy_paths:
  id_A = busy_path["id_A"]
  id_B = busy_path["id_B"]
  
  horz_prev = None

  if "path" in busy_path:
    for node in busy_path["path"]:
      # None
      if node["channel"] == 0: # vert constraint: id_A > node["node_id"]  >=  id_B , same horz
        if(node["node_id"] != id_B):
          constraints.append( vert_index[id_A] >= vert_index[node["node_id"]] +1)
          constraints.append( vert_index[node["node_id"]] >= vert_index[id_B])
        constraints.append( horz_index[id_A] == horz_index[node["node_id"]] )        

      if node["channel"] == 1: # vert constraint: id_A < node["node_id"]  <=  id_B , same horz
        if(node["node_id"] != id_B):
          constraints.append( vert_index[id_A]+1 <= vert_index[node["node_id"]])
          constraints.append( vert_index[node["node_id"]] <= vert_index[id_B])          
        constraints.append( horz_index[id_A] == horz_index[node["node_id"]] )        
      
      if node["channel"] == 2 or node["channel"] == 3: # horz constraint: id_A > node["node_id"]  >  id_B  or id_A < node["node_id"]  <  id_B
        if(node["node_id"] != id_B):
          invalidate =  cp.Variable( 1, boolean=True) 
          invalidate_rev =  cp.Variable( 1, boolean=True) 
          constraints.append( invalidate + invalidate_rev == 1)

          constraints.append( 100*invalidate + horz_index[id_A] >= horz_index[node["node_id"]] +1)
          constraints.append( 100*invalidate + horz_index[node["node_id"]] >= horz_index[id_B] +1)
          
          constraints.append(  horz_index[id_A]+1 <= horz_index[node["node_id"]] + 100*invalidate_rev)
          constraints.append(  horz_index[node["node_id"]]+1 <= horz_index[id_B] + 100*invalidate_rev  )
          

        if horz_prev is not None:
          constraints.append( vert_index[horz_prev] == vert_index[node["node_id"]] )        
        horz_prev = node["node_id"]

# cost = cp.sum(vert_group_indicator) + cp.sum(horz_group_indicator)
cost = 0
for i in range(N_VERT):
  cost += (i+1)*vert_group_indicator[i]
for j in range(N_HORZ):
  cost += (j+1)*horz_group_indicator[j]


assign_prob = cp.Problem(cp.Minimize(cost),constraints)

assign_prob.solve(solver=cp.CBC)

core_map = {}
max_vid = -1
max_hid = -1

for core_id in range(num_cores+len(deprecated)):
  if core_id in deprecated: continue
  vid = vert_pos.value[core_id,:].nonzero()[0][0]
  hid = horz_pos.value[core_id,:].nonzero()[0][0]
  
  core_map[(vid,hid)] = core_id
  # print(vid,hid,core_id)
  if(max_vid < vid) : max_vid =  vid
  if(max_hid < hid) : max_hid =  hid

print("",end='\t')
for hid in range(max_hid+1):
  print("({})".format(hid),end='\t')
print("")

for vid in range(max_vid+1): 
  print("({})".format(vid),end='\t') 
  for hid in range(max_hid+1):
    if(vid,hid) in core_map:
      print("{}".format(core_map[(vid,hid)]),end='\t')
    else:
      print("*",end='\t')
  print("") 

mapped = {}
mapped["ppin"] = path_info["ppin"]
mapped["map"] = []
mapped["CHA_to_os"] = path_info["CHA_to_os"]

horz_reverse = False
done = False
for hid in range(max_hid+1):  
  for vid in range(max_vid+1): 
    if(vid,hid) in core_map:
      done = True
      if core_map[(vid,hid)]  != 0:
        horz_reverse = True
      else: 
        horz_reverse = False
    if(done): break
  if(done): break


for hid in range(max_hid+1):
  if(horz_reverse):
    hid = max_hid - hid
  for vid in range(max_vid+1): 
    if(vid,hid) in core_map:
      mapped["map"].append(core_map[(vid,hid)])
    else:
      mapped["map"].append(-1)

fn_head = sys.argv[1][:sys.argv[1].rfind(".json")]
with open(fn_head+'.mapped.json', 'w') as outjson:
  json.dump(mapped, outjson)
