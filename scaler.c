
#include "scaler.h"
int scaleImg(SDL_Surface *src,SDL_Surface *dst,int scale){
	int* spix,*dpix;
	int sh,dh,sw,dw;
	int i,j;
	int dstptr,dstlin;
	spix=(int*)src->pixels;
	dpix=(int*)dst->pixels;
	sw=src->w;
	sh=src->h;
	//dw=dst->w;
	//dh=dst->h;
	dstptr=0;
	for(i=0;i<sh;i++){
		
		for(j=0;j<sw;j++){
			int px=spix[i*sw+j];
			dpix[dstptr]=px; dstptr++;
			dpix[dstptr]=px; dstptr++;
			
			
		}
		for(j=0;j<sw;j++){
			int px=spix[i*sw+j];
			dpix[dstptr]=px; dstptr++;
			dpix[dstptr]=px; dstptr++;
			
			
		}
		
		/*for(j=0;j<sw;j++){
			int px=spix[i*sh+j];
			dpix[dstlin+dstptr]=px; dstptr++;
			dpix[dstlin+dstptr]=px; dstptr++;
			
		}*/
		
	}
	
	
	return 0;
}