/*
 * Tastatureingaben auswerten
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include "bb.h"
#include "input.h"

/* exportierte Variablen */

char question[MAX_INP_LENGTH];
int InpMode;
int InpLength;
char input[MAX_INP_LENGTH];
unsigned long InpBoxPixel;
unsigned long InpLightPixel;
unsigned long InpDarkPixel;

/* moduleigene Variablen */

#define BorderWidth 3
#define TextDistance 4

XRectangle InpRect;
int InpOffs;
int FontHeight;

/* Dialogbox loeschen */
void ClearInpBox()
{
        XSetForeground(display,gc,BackgroundPixel);
	XFillRectangle(display,win,gc,InpRect.x-BorderWidth,InpRect.y-BorderWidth,
		       InpRect.width+BorderWidth*2,InpRect.height+BorderWidth*2);
	InpMode=0;
}

/* Eingabetext loeschen (bei Backspace und Winkel +,-,l,r Aenderung) */
void ClearInpText()
{
        XSetForeground(display,gc,InpBoxPixel);
        XFillRectangle(display,win,gc,InpRect.x+InpOffs,InpRect.y,
		       InpRect.width-InpOffs,InpRect.height);
}

/* wenn del oder backspace, dann ein Zeichen loeschen, wenn strlen()>0 */
void TestDel(KeySym keysym)
{
        int len;
        if ((keysym==XK_BackSpace) || (keysym==XK_Delete)) {
                if ( len=strlen(input) ) {
		        input[len-1]=0;
		        ClearInpText();
		}
	}
}

/* wenn noch Platz ist (InpLength), dann Zeichen an Input anhaengen */
void TryAdd()
{
        if ( strlen(input) + strlen(buffer)> InpLength)
	        XBell(display, 100); else strcat(input, buffer);
}

/* nur den Eingabetext neu zeichnen */
void DrawInpText()
{
        XSetForeground(display,gc,TextPixel);
        XDrawString(display,win,gc,
		    InpRect.x+InpOffs+TextDistance,InpRect.y+TextDistance+FontHeight,
		    input,strlen(input));
}

/* gesamte Dialogbox, inklusiv Frage und Eingabetext zeichen */
void DrawInpBox()
{
        InpRect.width=(strlen(question)+InpLength)*FontWidth+2*TextDistance;
        InpRect.height=FontHeight+2*TextDistance+font_info->max_bounds.descent;
        InpRect.x=(ScreenWidth-InpRect.width)/2;
        InpRect.y=BorderWidth+20;
        if (InpRect.x<BorderWidth) InpRect.x=BorderWidth;
        InpOffs=strlen(question)*FontWidth;

        XSetForeground(display,gc,InpBoxPixel);
	XFillRectangle(display,win,gc,InpRect.x,InpRect.y,InpRect.width,InpRect.height);

        XSetForeground(display,gc,InpLightPixel);
        XDrawLine(display,win,gc,InpRect.x-2,InpRect.y-2,
		  InpRect.x+InpRect.width+1,InpRect.y-2);
        XDrawLine(display,win,gc,InpRect.x-2,InpRect.y-1,
		  InpRect.x+InpRect.width,InpRect.y-1);
        XDrawLine(display,win,gc,InpRect.x-2,InpRect.y,
		  InpRect.x-2,InpRect.y+InpRect.height+1);
        XDrawLine(display,win,gc,InpRect.x-1,InpRect.y,
		  InpRect.x-1,InpRect.y+InpRect.height);

        XSetForeground(display,gc,InpDarkPixel);
        XDrawLine(display,win,gc,InpRect.x-1,InpRect.y+InpRect.height+1,
		  InpRect.x+InpRect.width+1,InpRect.y+InpRect.height+1);
        XDrawLine(display,win,gc,InpRect.x,InpRect.y+InpRect.height,
		  InpRect.x+InpRect.width+1,InpRect.y+InpRect.height);
        XDrawLine(display,win,gc,InpRect.x+InpRect.width+1,InpRect.y-1,
		  InpRect.x+InpRect.width+1,InpRect.y+InpRect.height-1);
        XDrawLine(display,win,gc,InpRect.x+InpRect.width,InpRect.y,
		  InpRect.x+InpRect.width,InpRect.y+InpRect.height-1);
        XDrawRectangle(display,win,gc,InpRect.x-3,InpRect.y-3,
		   InpRect.width+5,InpRect.height+5);

        XSetForeground(display,gc,TextPixel);
        XDrawString(display,win,gc,InpRect.x+TextDistance,
		    InpRect.y+TextDistance+FontHeight,
		    question,strlen(question));
        DrawInpText();
}

/* Eingabe auswerten und ggf. Inputbox loeschen */
void EvalInput(KeySym keysym)
{
        if ((InpMode!=JA_NEIN_INP) 
	    && ((keysym == XK_Return) || (keysym == XK_KP_Enter)
	    || (keysym == XK_Linefeed) )) {
	        ClearInpBox();
		return;
	}
	switch (InpMode) {
	    case INFO_INP:
	        if (((keysym >= XK_KP_Space) && (keysym <= XK_KP_9))
		     || ((keysym >= XK_space) && (keysym <= XK_asciitilde))) ClearInpBox();
	        break;
	    case JA_NEIN_INP:
	        if (keysym == XK_J || keysym == XK_j)  {
	                input[0]=1;
	                ClearInpBox();
	        } else   {
	                if (keysym == XK_N || keysym == XK_n)   {
		                input[0]=0;
		                ClearInpBox();
			}
		}
	        break;
	    case NUMBER_INP:
	        TestDel(keysym);
	        if ((keysym <= XK_9)&&(keysym >= XK_0) || (keysym <= XK_KP_9)
		    &&(keysym >= XK_KP_0)) TryAdd();
	        break;
	    case ANGLE_INP:
	        if (strlen(input)>2) TestDel(keysym);
	        if ((keysym==XK_L) || (keysym==XK_l)) {
		        input[0]='L';
		        ClearInpText();
		}
	        if ((keysym==XK_R) || (keysym==XK_r)) {
		        input[0]='R';
		        ClearInpText();
		}
	        if ((keysym==XK_plus) || (keysym==XK_KP_Add)) {
		        input[1]='+';
		        ClearInpText();
		}
	        if ((keysym==XK_minus) || (keysym==XK_KP_Subtract)) {
		        input[1]='-';
		        ClearInpText();
		}
	        if ((keysym <= XK_9)&&(keysym >= XK_0) || (keysym <= XK_KP_9)
		    &&(keysym >= XK_KP_0)) TryAdd();
	        break;
	    case TEXT_INP:
	        TestDel(keysym);
	        if (((keysym >= XK_KP_Space) && (keysym <= XK_KP_9)) 
		    || ((keysym>= XK_space)
		    && (keysym <= XK_asciitilde))) TryAdd();
	        break;
	}
}

