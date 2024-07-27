import sys
import numpy as np

N = 0

def doit1(id,pth):
  arr = np.load(pth)['points']
  print(np.shape(arr))
  centroid = np.mean(arr,axis=0)
  print(arr);
  arr -= centroid
  print(np.isfortran(arr));
  arr = np.ascontiguousarray(arr);
  print(np.isfortran(arr));
  a = arr
  np.save("data/lat_"+str(id).zfill(3)+".npy",arr);
  N = np.shape(arr)[0]

  f = open("conf.h","w")
  f.write(f'#define ID "{id}"\n#define N {N}\n')

def doit2(id,pth):
  arr = np.load(pth)['points']*1000
  np.save("data/pts_"+str(id).zfill(3)+".npy",arr);

def doit3(id,pth):
  arr = np.load(pth)['classifications']
  np.save("data/class_"+str(id).zfill(3)+".npy",arr);


doit1(sys.argv[1],"data/"+sys.argv[1]+"_lat.npz")
doit2(sys.argv[1],"data/"+sys.argv[1]+"_xyz.npz")
doit3(sys.argv[1],"data/"+sys.argv[1]+"_class.npz")

