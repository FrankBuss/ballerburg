/*
 * XConnect herstellen, Font laden, Window und Pixmap erstellen usw.
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>

#include "bb.h"
#include "bb_icon"
#include "colors"
#include "input.h"

void GetColors()
{
        Colormap cmap;
	XColor rgb;
	cmap=DefaultColormap(display, screen_num);
	
	rgb.red=BackgroundR; rgb.green=BackgroundG; rgb.blue=BackgroundB;
	if (!XAllocColor(display,cmap,&rgb)) 
            BackgroundPixel=WhitePixel(display,screen_num); else
	        BackgroundPixel=rgb.pixel;
	
	rgb.red=TerrainR; rgb.green=TerrainG; rgb.blue=TerrainB;
	if (!XAllocColor(display,cmap,&rgb))
	    TerrainPixel=BlackPixel(display,screen_num); else
	        TerrainPixel=rgb.pixel;
	
	rgb.red=ExplosionR; rgb.green=ExplosionG; rgb.blue=ExplosionB;
	if (!XAllocColor(display,cmap,&rgb))
	    ExplosionPixel=BlackPixel(display,screen_num); else
	        ExplosionPixel=rgb.pixel;
	
	rgb.red=ObjectR; rgb.green=ObjectG; rgb.blue=ObjectB;
	if (!XAllocColor(display,cmap,&rgb))
	    ObjectPixel=BlackPixel(display,screen_num); else
	        ObjectPixel=rgb.pixel;
	
	InpBoxPixel=TerrainPixel;
	InpLightPixel=ObjectPixel;
	InpDarkPixel=BlackPixel(display,screen_num);
	TextPixel=BlackPixel(display,screen_num);
}

void CreateWindow()
{
        int x=0,y=0;
	unsigned int borderwidth=4;
	XSetWindowAttributes attrib;
	unsigned long attribmask;
	attribmask = CWBackPixel | CWEventMask;
	attrib.background_pixel=BackgroundPixel;
	attrib.event_mask=ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask | PropertyChangeMask;
	win = XCreateWindow(display, RootWindow(display,screen_num), x, y, 
		     ScreenWidth, ScreenHeight, borderwidth, 
		     DefaultDepth(display, screen_num), InputOutput,
		     DefaultVisual(display, screen_num), attribmask, &attrib);
}

void SetIconAndSize()
{
        char *window_name = "Ballerburg";
        char *icon_name = "bb";
	XSizeHints size_hints;
        Pixmap icon_pixmap;
	icon_pixmap = XCreateBitmapFromData(display, win, bb_icon_bits, 
				     bb_icon_width, bb_icon_height);
	
	size_hints.flags = PPosition | PMinSize | PMaxSize;
	size_hints.min_width = 300;
	size_hints.min_height = 200;
	size_hints.max_width = 1000;
	size_hints.max_height = 500;
	XSetStandardProperties(display, win, window_name, icon_name, 
		       icon_pixmap, NULL, 0, &size_hints);
}

int LoadFont()
{
        char *fontname = "9x15";
	
	/* Load font and get font information structure. */
	if ((font_info = XLoadQueryFont(display,fontname)) == NULL)  {
	        (void) fprintf( stderr, "%s: Cannot open 9x15 font\n", progname);
		return 0;
	}
	FontHeight=font_info->max_bounds.ascent+font_info->max_bounds.descent;
	FontWidth=font_info->max_bounds.width;
        return 1;
}
	
void getGC()
{
	XGCValues values;
	values.background=BackgroundPixel;
	values.font=font_info->fid;
	gc = XCreateGC(display, win, GCBackground|GCFont, &values);
}

int init()
{
        char *display_name=NULL;
	if ( (display=XOpenDisplay(display_name)) == NULL )  {
	        (void) fprintf( stderr, "%s: cannot connect to X server %s\n", 
			   progname, XDisplayName(display_name));
	        return 0;
	}

        screen_num = DefaultScreen(display);
	ScreenWidth = 640, ScreenHeight = 350;

	GetColors();
	CreateWindow();
        if (LoadFont())  {
	        SetIconAndSize();
	        getGC();
                XMapWindow(display,win);
	} else return 0;
        return 1;
}
   
