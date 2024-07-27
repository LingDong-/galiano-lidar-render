#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include "conf.h"

float dx = 0.00000762939453125000000000000000000000000000000000;
float dy = 0.00000381469726562500000000000000000000000000000000;

float mx = INFINITY;
float my = INFINITY;
float mz = INFINITY;

float Mx = -INFINITY;
float My = -INFINITY;
float Mz = -INFINITY;

char hdr[128] = {0x93,0x4E,0x55,0x4D, 0x50,0x59,0x01,0x00, 0x76,0x00};

char conf[1024] = {0};

int main(){
  FILE* fp = fopen("data/lat_" ID ".npy","rb");
  fseek(fp,128,SEEK_SET);
  for (int i = 0; i < N; i++){
    float x,y,z;
    fread(&x,4,1,fp);
    fread(&y,4,1,fp);
    fread(&z,4,1,fp);
    mx = fmin(mx,x);
    my = fmin(my,y);
    mz = fmin(mz,z);
    Mx = fmax(Mx,x);
    My = fmax(My,y);
    Mz = fmax(Mz,z);
  }
  printf("%f %f\n",(Mx-mx)/dx,(My-my)/dy);


  sprintf(hdr+10,"{'descr': '<u2', 'fortran_order': False, 'shape': (%d, 2), }",N);

  FILE* fd = fopen("data/gridmap_" ID ".npy","wb");
  fseek(fp,0,SEEK_SET);
  for (int i = 0; i < 128; i++){
    fputc(hdr[i],fd);
  }

  fseek(fp,128,SEEK_SET);
  for (int i = 0; i < N; i++){
    float x,y,z;
    fread(&x,4,1,fp);
    fread(&y,4,1,fp);
    fread(&z,4,1,fp);

    float fx = (x-mx)/dx;
    float fy = (y-my)/dy;

    uint16_t ix = fx;
    uint16_t iy = fy;
    
    if (fx-ix || fy-iy){
      printf("%f %f %d %d\n",fx,fy,ix,iy);
    }
    fwrite(&iy,2,1,fd);
    fwrite(&ix,2,1,fd);
  }

  sprintf(conf,"#define ID \"%s\"\n#define N %d\n#define H %d\n#define W %d\n",ID,N,(int)((Mx-mx)/dx)+1,(int)((My-my)/dy)+1);

  FILE* fz = fopen("conf.h","w");
  int i=0;
  while (conf[i]){
    fputc(conf[i],fz);
    i++;
  }


  sprintf(conf,"%d\n%d\n",(int)((Mx-mx)/dx)+1,(int)((My-my)/dy)+1);
  fz = fopen("output/dim_" ID ".txt","w");
  i=0;
  while (conf[i]){
    fputc(conf[i],fz);
    i++;
  }

}