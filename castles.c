/*
 * 
 * PlaceCastles:
 * 
 * erstellt die Positionen der Burgen und die Spielerreihenfolge
 * 
 * 
 * DrawCastles:
 * 
 * zeichnet die Burgen
 * 
*/

#include "bb.h"
#include <stdio.h>

void GenerateOrder()
{
	int c,c2,z;
        for (c=0;c<NumPl;c++) OrderPl[c]=-1;
        for (c=0;c<NumPl;c++) {
                z=GetRandom(NumPl-c);
	        c2=-1;
        	while (z>=0) if (OrderPl[++c2]<0) z--;
	        OrderPl[c2]=c;
	}
}

void PlaceCastles()
{
        int c,b=1000/NumPl,a=b*RandomPlace/200+8,s=b-2*a;
        GenerateOrder();  /* fuer die Burgplazierungen */
        for (c=0;c<NumPl;c++)  {
                CastleX[OrderPl[c]]=12+a+c*b+GetRandom(s);
	        CastleY[OrderPl[c]]=Terrain[CastleX[OrderPl[c]]]-4;
	}
        GenerateOrder();  /* fuer die Spielerreihenfolge */
}

/*
 * Burg in Einheiten:
 *
 * ## ## ##
 * ## ## ##
 * ########
 * ########
 *  ######
 *  ######   
 *  ######
 *  ######
 *  ###0## <- Nullpunkt
 * 
*/
void DrawCastles()
{
        float sx=(float)ScreenWidth/1024, sy=(float)ScreenHeight/512;
        int c,x_offs,len,g_len;
        float x,y,unit=sx*3;
        if (unit==0) unit=1;
        XSetForeground(display, gc, ObjectPixel);
        for (c=0;c<NumPl;c++) {
	        if (PlayerLive[c])  {
	                x=sx*CastleX[c];
	                y=sy*CastleY[c];
	                XSetForeground(display, gc, BackgroundPixel);
	                XFillRectangle(display,win,gc,x-5*unit,y-30*unit,10*unit,31*unit);
	                XSetForeground(display,gc,TerrainPixel);
	                XFillRectangle(display,win,gc,x-4*unit,y+unit,8*unit,22*unit);
	                XSetForeground(display,gc,ObjectPixel);
	                XFillRectangle(display,win,gc,x-4*unit+.5,y-8*unit+.5,2*unit+.5,2*unit+.5);
	                XFillRectangle(display,win,gc,x-unit+.5,y-8*unit+.5,2*unit+.5,2*unit+.5);
                        XFillRectangle(display,win,gc,x+2*unit+.5,y-8*unit+.5,2*unit+.5,2*unit+.5);
                        XFillRectangle(display,win,gc,x-4*unit+.5,y-6*unit+.5,8*unit+.5,2*unit+.5);
                        XFillRectangle(display,win,gc,x-3*unit+.5,y-4*unit+.5,6*unit+.5,5*unit+.5);
	                XSetForeground(display, gc, TextPixel);
                        len=strlen(Name[c]);
	                g_len=font_info->max_bounds.width*len;
	                x_offs=x-g_len/2;
                        if (x_offs+g_len>ScreenWidth) x_offs=ScreenWidth-g_len;
	                if (x_offs<4) x_offs=4;
	                XDrawString(display,win,gc,x_offs,
				    ScreenHeight-font_info->max_bounds.ascent,Name[c],len);
		}
	}
}
