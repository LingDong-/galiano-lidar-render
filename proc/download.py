import sys
import os
links = [x.strip() for x in open("links.txt",'r').read().split('\n') if len(x.strip())];

cmds = []
for i in range(len(links)):
  name = links[i].split('/')[-1].split('.')[0].split("_xyes")[0].split("bc_")[1].replace('_','')
  cmds.append("curl "+links[i]+" > dl/"+name+".laz")

f = " &\n".join(cmds)
print(f)