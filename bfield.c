/*
 * 
 * MakeBattleField:
 * 
 * erzeugt eine neue Berglandschaft
 * 
 * 
 * DrawBattleField:
 * 
 * zeigt die Berglandschaft an
 * 
*/

#include "bb.h"
#include <stdio.h>

void MakeBattleField()
{
	int trend=0,c;
	Terrain[0]=460-GetRandom(170);
	for (c=1;c<1025;c++) {
		int inc=GetRandom(6),motion=GetRandom(20)+1,FirstHalf=0,ScreenPos=0;
		if (c<500) FirstHalf=1; else FirstHalf=0;
		if (GetRandom(4)+1==1) ScreenPos=1; else ScreenPos=0;
		if (1<=motion&&motion<=10) {
			if (ScreenPos&&FirstHalf) trend-=inc;
			if (ScreenPos&&!FirstHalf) trend+=inc; else {
			        if (motion<4+GetRandom(5)) trend-=inc; else trend+=inc;
			}
		}
		if (11<=motion&&motion<=14) {
			if (ScreenPos&&FirstHalf) trend-=inc*2*(GetRandom(4)+1);
                        if (ScreenPos&&FirstHalf) trend-=inc*2;
		        if (ScreenPos&&!FirstHalf) trend+=inc*2; else {
			        if (motion<11+GetRandom(8)) trend-=inc*2; else trend+=inc*2;
			}
		}
		switch (motion) {
		    case 15: trend=0; break;
		    case 16: trend=1; break;
		    case 17: trend=-1; break;
		}
	        if (trend<-10) Terrain[c]=Terrain[c-1]-3; else {
		        if (trend<0) Terrain[c]=Terrain[c-1]-1;
		}
	        if (trend>10) Terrain[c]=Terrain[c-1]+3; else {
		        if (trend>=0) Terrain[c]=Terrain[c-1]+1;
		}
		if (Terrain[c]>460) {
			Terrain[c]=460;
			if (FirstHalf) trend=-9; else trend=-3;
		} else {
			if (Terrain[c]<227) {
				Terrain[c]=227;
				if (FirstHalf) trend=9; else trend=3;
			}
		}
	}
        Wind=GetRandom(10)-5;
        if (GetRandom(3)==1)  {
                if (Wind>0) Wind+=GetRandom(10)+1; else Wind-=GetRandom(10)+1;
	}
}

void DrawWind(int x,int y,int c,int c2)
{
        XDrawLine(display,win,gc,x,y,x+c,y);
        XDrawLine(display,win,gc,x+c,y,x+c-c2,y-c2);
        XDrawLine(display,win,gc,x+c,y,x+c-c2,y+c2);
}

void DrawBattleField()
{
        int c,c2,x,y;
        XSetForeground(display, gc, TerrainPixel);
        for (c=0;c<ScreenWidth;c++)
            XDrawLine(display,win,gc,
		      c,(Terrain[(c<<10)/ScreenWidth]*ScreenHeight)>>9,
		      c,ScreenHeight-1);
        if (Wind) {
                c=Wind*(int)ScreenWidth/160;
                c2=ScreenWidth/160;
                x=ScreenWidth/2;
                y=c2+1;
		if (c<0) c2=-c2;
                DrawWind(x,y,c,c2);
	        DrawWind(x,y+1,c,c2);
	}
}
					     
