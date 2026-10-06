/*
 * Ballerburg V0.9, eine Umsetzung in C fuer X11
 * Copyright 7/1996 Frank Buss
 *
 * 
 * main.c:
 *
 * Verbindungsaufbau zum X-Server, Farben und Font holen,
 * Hauptwindow aufbauen, Event-Handling
 * 
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>

#include <stdio.h>
#include <signal.h>

#include "bb.h"
#include "input.h"

int end;
int screen_num;
Window win;
GC gc;
Display *display;
XFontStruct *font_info;
int FontWidth, FontHeight;
char buffer[buf_len+1];
char *progname;

/* Ballerburg spezifische Variablen */

int NumGames;
int InGame;
int InitGame;
int GameState;
int PlayerCount;
int NumPl;
int OrderPl[MaxPlayer];
int PlayerLive[MaxPlayer];
int PlayerWin[MaxPlayer];
char Name[MaxPlayer][20];
int CastleX[MaxPlayer];
int CastleY[MaxPlayer];
float pi;

unsigned int ScreenWidth,ScreenHeight;

int Terrain[1025];
int Wind;
float Gravity;

unsigned long BackgroundPixel;
unsigned long TerrainPixel;
unsigned long ExplosionPixel;
unsigned long ObjectPixel;
unsigned long TextPixel;

int RandomPlace;
int delay;
int SoundMode;
int GameMode;
int CastleCraterRadius;
int ShotCraterRadius;
int MountainPermeability;
int server;
int client;
int NumCl;  /* Anzahl Clienten bei Serverbetrieb */
char *ServerName;
extern id;
int flag;

/* wenn ein expose aufgetreten ist, neu zeichnen */
void Redraw()
{
        XClearWindow(display,win);
        if (GameState)  {
                DrawBattleField();
                DrawCastles();
        }
       	if (InpMode) DrawInpBox();
	XFlush(display);
}

void PrintUsage()
{
        printf("Usage: bb [OPTION]\n");
        printf("  -h|?       this page\n");
	printf("  -r number  set random for placing castles in per cent ( default 70)\n");
	printf("  -d number  set delay-time (0=disable) (default 10)\n");
	printf("  -m number  sound-mode: 0=no sound, 1=beeper, 2=/dev/audio (default 2)\n");
	printf("  -g number  game-mode: 0=count winning rounds, 1=count hits (default 1)\n");
        printf("  -a number  cAstle: set radius of crater (default 40)\n");
        printf("  -o number  shOt: set radius of crater (default 20)\n");
        printf("  -p number  permeability of mountain for shots (default 5)\n");
	printf("  -c server  run bb as client, connect to server\n");
        printf("  -s c r     run bb as server: 'c' clients; 'r' rounds\n");
}

void EventLoop()
{
        XEvent report;
        KeySym keysym;
        XComposeStatus compose;
        int c;
        while (!end) {
                XNextEvent(display, &report);
                switch  (report.type)  {
                    case Expose:
                        if (report.xexpose.count != 0) break;
                        Redraw();
                        break;
                    case KeyPress:
                        c=XLookupString((XKeyPressedEvent *)&report,buffer,
					buf_len,&keysym,&compose);
                        buffer[c]=0;
                        if (keysym == XK_Escape) end=1;
                        if (InpMode) {
	                        EvalInput(keysym);
	                        if (InpMode) DrawInpText();
			}
                        break;
                    case ConfigureNotify:
                        ScreenWidth=report.xconfigure.width;
                        ScreenHeight=report.xconfigure.height;
                        break;
                    case PropertyNotify:
                        break;
                    default:
                        break;
		}
                if (!InpMode) {
                        if (GameState) OnGame(); else OnInit();
		}
	}
}

int main(int argc, char **argv)
{
        int c;

        server=0;
        client=0;
	RandomPlace=30;
        delay=10;
	SoundMode=2;
	GameMode=1;
        CastleCraterRadius=40;
        ShotCraterRadius=20;
	MountainPermeability=5;
        for (c=1;c<argc;c++) {
                if (argv[c][0]=='-') {
                        switch (argv[c][1]) {
	                    case '?':
	                    case 'h':
                                PrintUsage();
	                        return 1;
			    case 'r':
			        if (++c<argc) RandomPlace=atoi(argv[c]); else {
			                PrintUsage();
			                return -1;
			        }
				if (RandomPlace<0||RandomPlace>100) {
				        PrintUsage();
					return -1;
				}
				RandomPlace=100-RandomPlace;
			        break;
			    case 'd':
			        if (++c<argc) delay=atoi(argv[c]); else {
			                PrintUsage();
			                return -1;
			        }
			        break;
			    case 'm':
			        if (++c<argc) SoundMode=atoi(argv[c]); else {
			                PrintUsage();
			                return -1;
			        }
				if (SoundMode<0||SoundMode>2) {
				        PrintUsage();
					return -1;
				}
			        break;
			    case 'g':
			        if (++c<argc) GameMode=atoi(argv[c]); else {
			                PrintUsage();
			                return -1;
			        }
				if (GameMode<0||GameMode>1) {
				        PrintUsage();
					return -1;
				}
			        break;
			    case 'a':
		                if (++c<argc) CastleCraterRadius=atoi(argv[c]); else {
			                PrintUsage();
				        return -1;
			        }
			        break;
			    case 'o':
			        if (++c<argc) ShotCraterRadius=atoi(argv[c]); else {
	                                PrintUsage();
		                        return -1;
			        }
			        break;
			    case 'p':
			        if (++c<argc) MountainPermeability=atoi(argv[c]); else {
				        PrintUsage();
					return -1;
				}
			        break;
			    case 'c':
			        if (++c<argc) ServerName=argv[c]; else {
			                PrintUsage();
			                return -1;
			        }
			        client=1;
			        break;
			    case 's':
			        if (++c<argc) server=atoi(argv[c]); else {
			                PrintUsage();
			                return -1;
			        }
      			        if (++c<argc) NumGames=atoi(argv[c]); else {
			                PrintUsage();
			                return -1;
			        }
			        if (NumGames<1) {
			                PrintUsage();
			                return -1;
			        }
			        break;
			    default:
			        PrintUsage();
			        return -1;
			}
		} else {
		        PrintUsage();
		        return -1;
		}
	}
   
        end=0;
        progname=argv[0];
        font_info=NULL;
        display=NULL;
        InitRandom();
        pi=3.141592654;
        Gravity=9.81;
        if (server) {
	        ServerInit(server);
	        ServerLoop();
	        return 1;
	}
        if (init()) {
	        if (client) {
		        ClientInit();
			ClearBuf('i');
			MyWrite(Sock);
		}
                GameState=0;
                if (!client) InitNumPl(); else InpMode=0;
                EventLoop();
	}
        if (font_info) {
	        XUnloadFont(display, font_info->fid);
	        XFreeGC(display,gc);
	}
        if (display) XCloseDisplay(display);
	if (id) kill(id,SIGTERM);  /* kill(0) wuerde die ganze Prozessgruppe beenden */
        return 1;
}
