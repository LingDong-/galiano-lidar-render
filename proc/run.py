import sys
import os
links = [x.strip() for x in open("links.txt",'r').read().split('\n') if len(x.strip()) and x[0]!='#'];

# print(links)

for i in range(len(links)):
  print(links[i])
  name = links[i].split('/')[-1].split('.')[0].split("_xyes")[0].split("bc_")[1].replace('_','')
  os.system("sh runconvert.sh "+name+"; sh runproc.sh "+name)
  # break;