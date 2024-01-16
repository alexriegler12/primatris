#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "imageloader.h"
#include "audio.h"
#include "scaler.h"
/*#include "testlevel.h"*/
SDL_Surface *phys;
SDL_Surface *screen;
SDL_Surface *scalesurf;
    SDL_Surface *overlay;
	SDL_Surface *tiles;
	SDL_Surface *backg;
	SDL_Surface *startscr;
	SDL_Surface *govr;
    SDL_Event event;
	int scale=1;
enum scene{
	SCN_STRT,
	SCN_PLAY,
	SCN_GOVR
	
};

typedef struct point{
	int x,y;
	
}Point;

typedef struct piece{
	char coords[4][4];
	int tile;
}Piece;

Piece pieces[7]={
	{ { {4,5,6,7},{1,5,9,13},{4,5,6,7},{1,5,9,13} },1 },/*I*/
	{ { {1,2,5,6},{1,2,5,6},{1,2,5,6},{1,2,5,6} },2 },/*O*/
	{ { {2,3,5,6},{1,5,6,10},{2,3,5,6},{1,5,6,10} },3 },/*S*/
	{ { {1,2,6,7},{2,5,6,9},{1,2,6,7},{2,5,6,9} },4 },/*Z*/
	{ { {1,2,3,7},{2,6,9,10},{1,5,6,7},{1,2,5,9} },5 },/*J*/
	{ { {1,2,3,5},{1,5,9,10},{3,5,6,7},{1,2,6,10} },6 },/*L*/
	{ { {1,2,3,6},{2,5,6,10},{2,5,6,7},{1,5,6,9} },7 }/*T*/
};
#define W 10
#define H 24
#define DELAY 1000;
int field[24][10];
Point current[4],next[4];
int curpc,nextpc,currot;
int needNew;
int currentTile,nextTile;
Point curPos;
int lastTime,debounceTime;
int curTime;
int up,down,left,right,start;
int inputStateChanged;
enum scene curScn;
void clrField(void){
	int i,j;
	for(i=0;i<H;i++){
		for(j=0;j<W;j++){
			field[i][j]=0;
		}
		
	}
	
}
void setPiece(Point* pie,int num,int rot,int *ptile){
		int i;
		for(i=0;i<4;i++){
				pie[i].x=(pieces[num].coords[rot][i])%4;
				pie[i].y=(pieces[num].coords[rot][i])/4;
		}
		*ptile=pieces[num].tile;
}
int checkColAndBounds(int xoff,int yoff){
	int i;
	for(i=0;i<4;i++){
		if(current[i].x+curPos.x+xoff>=W||current[i].x+curPos.x+xoff<0||current[i].y+curPos.y+yoff<0||current[i].y+curPos.y+yoff>=H) return 1;
		if(field[current[i].y+curPos.y+yoff][current[i].x+curPos.x+xoff]!=0) return 1;
	}
	return 0;
}
void writeCurrentToField(){
	int i;
	for(i=0;i<4;i++){
		field[current[i].y+curPos.y][current[i].x+curPos.x]=currentTile;
	}
}
void deleteLineAndLowerMap(int line){
	int i,j;
	for(i=0;i<W;i++){/*Clear Line*/
		field[line][i]=0;
	}
	for(i=0;i<W;i++){/*Clear First Line*/
		field[0][i]=0;
	}
	if(line>0){
		for(i=line-1;i>=0;i--){/*Lower the upper blocks*/
			for(j=0;j<W;j++){
				field[i+1][j]=field[i][j];
			}
		
		}
	}
	
}
void checkLinesAndDeleteFull(void){
	int i,j,count;
	for(i=H-1;i>=0;i--){
		count=0;
		for(j=0;j<W;j++){
			if(field[i][j]!=0) count++;
		}
		if(count==W){/*Full Line detected*/
			deleteLineAndLowerMap(i);
		}
	}
	
}

void calculateRectFromBlockNum(int num,int mapw, int blockd,SDL_Rect* r){
		r->x=(num%mapw)*blockd;
		r->y=(num/mapw)*blockd;
		r->w=blockd;
		r->h=blockd;
}


void drawField(int xoff,int yoff){
	int i,j;
	SDL_Rect srcr,dstr;
	for(i=4;i<H;i++){
		for(j=0;j<W;j++){
			calculateRectFromBlockNum(field[i][j],8,8,&srcr);
			dstr.x=xoff+j*8;
			dstr.y=yoff+(i-4)*8;
			SDL_BlitSurface( tiles, &srcr, screen, &dstr );
		}
	}
}
void drawPiece(int xoff,int yoff,Point* piece,int tile){
	int i;
	SDL_Rect srcr,dstr;
	calculateRectFromBlockNum(tile,8,8,&srcr);
	for(i=0;i<4;i++){
		dstr.x=xoff+piece[i].x*8;
		dstr.y=yoff+piece[i].y*8;
		SDL_BlitSurface( tiles, &srcr, screen, &dstr );
	}
	
}
void drawCurrentPiece(int xoff, int yoff){
	drawPiece(xoff+curPos.x*8,yoff+(curPos.y-4)*8,current,currentTile);
}
void selectRandomPiece(int firstTime){
	if(firstTime){
		nextpc=(int)rand()%7;
	}
	curpc=nextpc;
	currot=0;
	setPiece(current,curpc,0,&currentTile);
	
	curPos.x=3;
	curPos.y=4;
	nextpc=(int)rand()%7;
	setPiece(next,nextpc,0,&nextTile);
	
}
void rotateCurrent(int amount){
	if(amount==1){
		currot++;
		if(currot>3) currot=0;
	}
	if(amount==-1){
		currot--;
		if(currot<0) currot=3;
	}
	
	setPiece(current,curpc,currot,&currentTile);
		
	

}



void fill_audio(void *udata, Uint8 *stream, int len){
        /* Only play if we have data left */
        memset(stream,0,len);
		getMusic((short*)stream,len/4);
}




SDL_AudioSpec wanted;
int main(int argc __attribute__((unused)), char **argv)
{
	int ticktime=1000;
    //SDL_Window *window;
    
    srand(time(0));
    if(SDL_Init(SDL_INIT_EVERYTHING)) {
        fprintf(stderr,"SDL error %s\n", SDL_GetError());
        return 2;
    }
	
#ifdef _3ds
	SDL_N3DSKeyBind(KEY_START,SDLK_SPACE);




#endif	
	
	
	
	
	
    wanted.freq = 22050;
    wanted.format = AUDIO_S16;
    wanted.channels = 2;    /* 1 = mono, 2 = stereo */
    wanted.samples = 1024;  /* Good low-latency value for callback */
    wanted.callback = fill_audio;
    wanted.userdata = NULL;
	loadMusic("tetris.mod");
    /* Open the audio device, forcing the desired format */
    if ( SDL_OpenAudio(&wanted, NULL) < 0 ) {
        fprintf(stderr, "Audio Error: %s\n", SDL_GetError());
        return(-1);
    }
	SDL_PauseAudio(0);
    overlay=loadQoi("tetrovl.qoi");
	tiles=loadQoi("tetrtil.qoi");
	backg=loadJpeg("tetrbg.jpg");
	govr=loadJpeg("tetrgovr.jpg");
	startscr=loadJpeg("tetrstrt.jpg");
    //window = SDL_CreateWindow("SSFN normal renderer bitmap font test",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,800,600,0);
	
	
	if(scale>1){
		phys = SDL_SetVideoMode(320*scale,240*scale,32,SDL_SWSURFACE);
		
		screen=SDL_CreateRGBSurface(0,320,240,32,0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);
		scalesurf=SDL_CreateRGBSurface(0,320*scale,240*scale,32,0x000000FF,0x0000FF00,0x00FF0000,0xFF000000);
		
		
	}else{
		phys = SDL_SetVideoMode(320,240,32,SDL_SWSURFACE);
		screen=phys;
	}
    
    //memset(screen->pixels, 0xF8, screen->pitch*screen->h);
	curScn=SCN_STRT;
	lastTime=SDL_GetTicks();
	//setCurrent(0,0);
	
	while(1){
		/*timeSinceLast+=SDL_GetTicks();*/
		while (SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_KEYDOWN:
					inputStateChanged=1;
					switch(event.key.keysym.sym){
						case SDLK_RIGHT:
							right=1;
							break;
						case SDLK_LEFT:
							left=1;
							break;
						case SDLK_UP:
							up=1;
							break;
						case SDLK_DOWN:
							down=1;
							break;
						case SDLK_SPACE:
							start=1;
							break;
						
					}
					break;
				case SDL_KEYUP:
					inputStateChanged=1;
                    switch(event.key.keysym.sym){
						case SDLK_RIGHT:
							right=0;
							break;
						case SDLK_LEFT:
							left=0;
							break;
						case SDLK_UP:
							up=0;
							break;
						case SDLK_DOWN:
							down=0;
							break;
						case SDLK_SPACE:
							start=0;
							break;
						
					}
					break;    
                       

                    
                case SDL_QUIT:
                    SDL_Quit();
                    break;
				 
				 
				 
				 
			}
			
			 
			 
			 
			 
		}
		/*LOGIC*/
		curTime=SDL_GetTicks();
		if(curScn==SCN_PLAY){
		if(needNew){
			
			selectRandomPiece(0);
			if(checkColAndBounds(0,0)){
				curScn=SCN_GOVR;
			}
			needNew=0;
		}
		
		
		
		
		
		if(curTime>lastTime+ticktime){/*Tick*/
			
			
			if(checkColAndBounds(0,1)){
				writeCurrentToField();
				checkLinesAndDeleteFull();
				needNew=1;
			}else{
				curPos.y++;	
			}
				
				
			lastTime=curTime;
		}
		if(curTime>debounceTime+150||inputStateChanged){
			if(left){
				if(!checkColAndBounds(-1,0)){
					curPos.x--;
				}
			
			}
			if(right){
				if(!checkColAndBounds(1,0)){
					curPos.x++;
				}
			
			}
			if(down){
				/*if(!checkColAndBounds(0,1)){
					curPos.y++;
				}*/
				ticktime=75;
				
			}else{
				
				ticktime=1000;
			}
			if(up){
				
				
				rotateCurrent(1);
				if(checkColAndBounds(1,0)){
					rotateCurrent(-1);
				}
				
			}
			inputStateChanged=0;
			debounceTime=curTime;
		}
		
		
		/*checkLinesAndDeleteFull();*/
		/*ENDLOGIC*/
		/*SDL_BlitSurface( backg, NULL, screen, NULL );*/
		SDL_BlitSurface( overlay, NULL, screen, NULL );
		drawField(112,48);
		
		/*drawPiece(32,32,current,currentTile);*/
		drawPiece(224,80,next,nextTile);
		drawCurrentPiece(112,48);
		}
		if(curScn==SCN_STRT){
			if(curTime>debounceTime+150){
				if(start){
					curScn=SCN_PLAY;
					SDL_BlitSurface( backg, NULL, screen, NULL );
					clrField();
					selectRandomPiece(1);
					debounceTime=curTime;
				}
				
			}
			if(curScn==SCN_STRT)
				SDL_BlitSurface( startscr, NULL, screen, NULL );
			
			
		}
		if(curScn==SCN_GOVR){
			if(curTime>debounceTime+150){
				if(start){
					curScn=SCN_STRT;
					debounceTime=curTime;
				
				}
				
			}
			SDL_BlitSurface( govr, NULL, screen, NULL );
			
		}
		
		
		
		
		
		
		
		
		if(scale>1){
			
			scaleImg(screen,scalesurf,2);
			SDL_BlitSurface( scalesurf, NULL, phys, NULL );
		
		}
		SDL_Flip(phys);

	
	}
	return 0;
	/*for(int i=0;i<LEVEL_HEIGHT;i++){
		for(int j=0;j<LEVEL_WIDTH;j++){
			
			calculateRectFromBlockNum(level[i*LEVEL_WIDTH+j],8,16,&srcr);
			
			dstr.x=(j*16);
			dstr.y=(i*16)-400;
			if(srcr.x!=0||srcr.y!=0){
				printf("Block coords in tilemap: x:%i y:%i\n",srcr.x,srcr.y);
				printf("Block coords in frame: x:%i y:%i\n",dstr.x,dstr.y);
			}
			SDL_BlitSurface( tiles, &srcr, screen, &dstr );
		}
	}*/
    
	/*field[5][5]=2;
	field[10][5]=2;
	field[4][0]=2;*/
	
    //Update Screen
    //SDL_Flip( screen );
    //do_test(screen, argv[1]);
    
    /*do{ SDL_UpdateWindowSurface(window); SDL_Delay(10); } while(SDL_WaitEvent(&event) && event.type != SDL_QUIT &&
       event.type != SDL_MOUSEBUTTONDOWN && event.type != SDL_KEYDOWN);*/

    //SDL_DestroyWindow(window);
    //SDL_Quit();
    
}