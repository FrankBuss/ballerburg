#ifndef _bb_h_
#define _bb_h_

/* zeigt Debugmeldungen an */
/* #define DEBUG 1 */

#include <X11/Xlib.h>

/* globale X Variablen */

#define buf_len 100

extern Display *display;
extern int screen_num;
extern Window win;
extern GC gc;
extern XFontStruct *font_info;
extern int FontWidth, FontHeight;
extern char *progname;
extern char buffer[buf_len+1];


/* globale Ballerburg Variablen */

#define MaxPlayer 4
#define CastleWidth 10

extern int NumGames;     /* Anzahl Spiele */
extern int InGame;       /* aktuelles Spiel (countdown to 0) */
extern int InitGame;     /* aktuelle Initphase */
extern int GameState;    /* aktuelle Spielphase */
extern int PlayerCount;  /* Nummer des aktuellen Spielers */
extern int NumPl;        /* Anzahl Spieler */
extern int OrderPl[MaxPlayer];     /* Reihenfolge der Spieler */
extern int PlayerLive[MaxPlayer];  /* Lebensflag */
extern int PlayerWin[MaxPlayer];   /* Anzahl Gewinne eines Spielers */
extern char Name[MaxPlayer][20];   /* Spielernamen */
extern int CastleX[MaxPlayer];     /* x-Positionen der Burgen */
extern int CastleY[MaxPlayer];     /* y-Positionen der Burgen */
extern float pi;

extern unsigned int ScreenWidth,ScreenHeight;

extern int Terrain[1025];
extern int Wind;
extern float Gravity;

extern unsigned long BackgroundPixel;
extern unsigned long TerrainPixel;
extern unsigned long ExplosionPixel;
extern unsigned long ObjectPixel;
extern unsigned long TextPixel;

extern int RandomPlace;
extern int delay;
extern int SoundMode;
extern int GameMode;
extern int CastleCraterRadius;
extern int ShotCraterRadius;
extern int MountainPermeability;
extern int server;
extern int client;
extern char *ServerName;
extern int Sock;
extern int flag;

/* Funktionen */

extern void MakeBattleField();
extern void DrawBattleField();
extern int GetRandom();
extern void InitRandom();
extern void DrawInpBox();
extern void ProcessInput();
extern int init();
extern void sleepms();
extern int end;

#endif
