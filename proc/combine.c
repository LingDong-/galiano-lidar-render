#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif


#include "conf.h"

int nx = 0;
int ny = 0;

char hdr[128] = {0x93,0x4E,0x55,0x4D, 0x50,0x59,0x01,0x00, 0x76,0x00};

float* ga;
float* gb;

int main(){
  FILE* fp = fopen("data/gridmap_" ID ".npy","rb");
  fseek(fp,128,SEEK_SET);
  for (int i = 0; i < N; i++){
    uint16_t ix,iy;
    fread(&ix,2,1,fp);
    fread(&iy,2,1,fp);
    nx = MAX(nx,ix);
    ny = MAX(ny,iy);
  }
  nx++;
  ny++;
  printf("%d %d\n",nx,ny);

  ga = malloc(sizeof(float)*nx*ny*3);
  gb = malloc(sizeof(float)*nx*ny*3);
  for (int i = 0; i < nx*ny*3; i++){
    ga[i] = -INFINITY;
    gb[i] = -INFINITY;
  }

  sprintf(hdr+10,"{'descr': '<f4', 'fortran_order': False, 'shape': (%d, %d, 3), }",ny,nx);


  FILE* fq = fopen("data/class_" ID ".npy","rb");
  FILE* fr = fopen("data/pts_" ID ".npy","rb");


  fseek(fq,128,SEEK_SET);
  fseek(fr,128,SEEK_SET);
  fseek(fp,128,SEEK_SET);

  for (int i = 0; i < N; i++){
    float x,y,z;
    fread(&x,4,1,fr);
    fread(&y,4,1,fr);
    fread(&z,4,1,fr);

    uint16_t ix,iy;
    fread(&ix,2,1,fp);
    fread(&iy,2,1,fp);

    uint8_t c;
    fread(&c,1,1,fq);

    

    if (c == 1 /*&& ix > 0*/){
      // printf("%d\n",c);
      if (z > ga[iy*nx+ix+2]){
        ga[(iy*nx+ix)*3+0] = x;
        ga[(iy*nx+ix)*3+1] = y;
        ga[(iy*nx+ix)*3+2] = z;
      }
    }else if (c == 2){
      // printf("%d\n",c);
      if (z > gb[iy*nx+ix+2]){
        gb[(iy*nx+ix)*3+0] = x;
        gb[(iy*nx+ix)*3+1] = y;
        gb[(iy*nx+ix)*3+2] = z;
      }
    }
  }

  FILE* fa = fopen("data/veg0_" ID ".npy","wb");
  FILE* fb = fopen("data/gnd0_" ID ".npy","wb");
  fseek(fp,0,SEEK_SET);
  for (int i = 0; i < 128; i++){
    fputc(hdr[i],fa);
    fputc(hdr[i],fb);
  }
  for (int i = 0; i < ny; i++){
    for (int j = 0; j < nx; j++){
      for (int k = 0; k < 3; k++){
        float a = ga[(i*nx+j)*3+k];
        float b = gb[(i*nx+j)*3+k];
        fwrite(&a,4,1,fa);
        fwrite(&b,4,1,fb);
      }
    }
  }

}