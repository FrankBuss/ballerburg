/*
 * Routinen die waehrend des Spiels abgearbeitet werden
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "bb.h"
#include "state.h"
#include "input.h"
#include "sound.h"

int winner;
int angle;
int velocity;

void SetAngle()
{
        int x,left,c;
        strcpy(question, "Spieler ");
        strcat(question, Name[OrderPl[PlayerCount]]);
        strcat(question, ": Winkel: ");
	InpLength=5;
	InpMode=ANGLE_INP;
	strcpy(input," +");
        /* Die Richtung initialisieren, in der die meisten Burgen stehen */
        left=0;
        x=CastleX[OrderPl[PlayerCount]];
        for (c=0;c<NumPl;c++) {
                if (CastleX[c]!=x&&PlayerLive[c]) {
                        if (CastleX[c]<x) left++; else left--;
                }
        }
        if (left>0) input[0]='L'; else input[0]='R';
        GameState=GAME_SET_ANGLE;
        DrawInpBox();
}

void SetVelocity()
{
        strcpy(question, "Spieler ");
        strcat(question, Name[OrderPl[PlayerCount]]);
        strcat(question, ": Kraft: ");
	InpLength=4;
	InpMode=NUMBER_INP;
 	input[0]=0;
        GameState=GAME_SET_VELOCITY;
	DrawInpBox();
}

void InitRound()
{
        int c;
        for (c=0;c<NumPl;c++) PlayerLive[c]=1;  /* aktuelles Lebensflag */
        XSetForeground(display, gc, BackgroundPixel);
        XFillRectangle(display, win, gc, 0, 0, ScreenWidth, ScreenHeight);
        PlayerCount=0;
        if (!client) {
	        MakeBattleField();
                PlaceCastles();
                SetAngle();
	} else GameState=99;
        Redraw();
}

/* zieht zufaellig Teile vom Berg ab */
void RandomSubtract(int x,int rad)
{
        int c;
        if (delay) sleepms(200);
        for (c=0;c<rad;c++) {
	        if (x+c<1025) {
	                Terrain[x+c]+=GetRandom(rad-c)+c/2;
	                if (Terrain[x+c]>460) Terrain[x+c]=460;
	        }
	        if (x-c>=0) {
	                Terrain[x-c]+=GetRandom(rad-c)+c/2;
	                if (Terrain[x-c]>460) Terrain[x-c]=460;
	        }
        }
}

/* liefert -2 bei Hintergrund, -1 bei Terrain und 0..NumPl bei Treffer */
unsigned long GetPixel(int x, int y)
{
        int c,dx,dy;
        for (c=0;c<NumPl;c++) {
                if (PlayerLive[c]) {
                        dx=CastleX[c]-x;
                        dy=CastleY[c]-y;
		        if (dx>-14&&dx<19&&abs(dy-7)<14) return c;
		}
        }
        if (Terrain[x]<y) return -1;
        return -2;
}

void Shot(int x, int y)
{
        XFillArc(display,win,gc,((x*ScreenWidth)>>10)-ScreenWidth/200,((y*ScreenHeight)>>9)-ScreenWidth/200,
		 ScreenWidth/100,ScreenWidth/100,0,64*360);
}

void PutShot(int x, int y)
{
        if (x>3&&x<1021&&y>3&&y<509) {
                XSetForeground(display,gc,ObjectPixel);
	        Shot(x,y);
	}
}

void ClearShot(int x, int y)
{
        if (x>3&&x<1021&&y>3&&y<509) {
	        XSetForeground(display,gc,BackgroundPixel);
	        Shot(x,y);
	}
}

void DoExplosion(int x, int y)
{
        int c;
	if (!server) {
	        play(GROUND);
                XSetForeground(display,gc,ExplosionPixel);
                for (c=150;c>100;c-=8) {
                        XFillArc(display,win,gc,((x*ScreenWidth)>>10)-ScreenWidth/c,
			     ((y*ScreenHeight)>>9)-ScreenWidth/c,
		               ScreenWidth*2/c,ScreenWidth*2/c,0,64*360);
                        XFlush(display);
                        if (delay) sleepms(delay);
		}
                if (delay) sleepms(100);
                XSetForeground(display,gc,BackgroundPixel);
                XFillArc(display,win,gc,((x*ScreenWidth)>>10)-ScreenWidth/90,
		         ((y*ScreenHeight)>>9)-ScreenWidth/90,
		           ScreenWidth/45,ScreenWidth/45,0,64*360);
                XFlush(display);
	}
        if (!client) RandomSubtract(x,ShotCraterRadius);
}

void FillExplodeCastle(int x,int y)
{
        int c;
        for (c=100;c>10;c-=5) {
                XFillArc(display,win,gc,((x*ScreenWidth)>>10)-ScreenWidth/c,
			 ((y*ScreenHeight)>>9)-ScreenWidth/c,
		         ScreenWidth*2/c,ScreenWidth*2/c,0,64*360);
                XFlush(display);
                if (delay) sleepms(delay);
        }
}

void ExplodeCastle(int pl)
{
        int x=CastleX[pl],y=CastleY[pl];
	if (!server) {
	        play(DESTROY);
                XSetForeground(display,gc,ExplosionPixel);
                FillExplodeCastle(x,y);
                XSetForeground(display,gc,BackgroundPixel);
                FillExplodeCastle(x,y);
	}
        if (!client) RandomSubtract(x,CastleCraterRadius);
}

void SetGameInfo()
{
        ++InGame;
        sprintf(question,"Spieler %s hat Runde %d gewonnen.",Name[winner],InGame);
	InpLength=0;
	InpMode=INFO_INP;
	input[0]=0;
	GameState=GAME_INFO;
	DrawInpBox();
}

/* zeichnet den Schuss, liefert -1 wenn kein Treffer sonst Nummer des
 * getroffenen Spielers */
int PlotShot()
{
        float ang=(float)angle/(float)180*pi;
	float vel=(float)velocity;
	float win=(float)Wind;
	float InitialXVelocity=cos(ang)*vel;
	float InitialYVelocity=sin(ang)*vel;
        int xc=CastleX[OrderPl[PlayerCount]]-2;
        int yc=CastleY[OrderPl[PlayerCount]]-33;
	int DirectHit=2;
	int Impact=MountainPermeability;
	int OnScreen=1;
	int PlayerHit=0;
	int NeedErase=0;
        int flag;
	float t=0;
	int x,y,c;
	if (!server) play(SHOT);
        while (OnScreen) {
                if (!server) ClearShot(x,y);
                x=(int)((float)xc+InitialXVelocity*t+.5*win/5*t*t);
                y=(int)((float)yc-InitialYVelocity*t+.5*Gravity*t*t);
                if (!server) PutShot(x,y);
                flag=GetPixel(x,y);
                if (flag==-1) if (!--Impact) OnScreen=0;
                if (flag>=0) if (!--DirectHit) OnScreen=0;
                if (y>509||x<3||x>1021) OnScreen=0;
                t+=.1;
	        if (!server) {
		        XFlush(display);
				                if (delay) sleepms(delay);
		}
	}
	if (!Impact) DoExplosion(x,y);
	if (!DirectHit) {
	        ExplodeCastle(flag);
		return flag;
	}
	return -1;
}

/* deaktiviert Spieler flag; liefert 1 zurueck wenn ein Spieler gewonnen hat
 * und erhoeht dann auch PlayerWin[] und setzt winner auf diese Spielernummer */
int CheckWinner(int flag)
{
        int c,last,alive;
	PlayerLive[flag]=0;
	alive=0;
	if (GameMode==1&&OrderPl[PlayerCount]!=flag) PlayerWin[OrderPl[PlayerCount]]++;
	for (c=0;c<NumPl;c++) {
	        if (PlayerLive[c]) {
		        last=c;
			alive++;
		}
	}
	if (alive==1) {
                if (GameMode==0) PlayerWin[last]++;
		winner=last;
		return 1;
	}
	return 0;
}

void SetGameEnd()
{
        int c;	     
	strcpy(question,"Ende. ");
	for (c=0;c<NumPl;c++)
            sprintf(&question[strlen(question)],"%s: %d  ",Name[c],PlayerWin[c]);
	InpLength=0;
	InpMode=INFO_INP;
	GameState=GAME_END;
	DrawInpBox();
}

/* naechsten noch lebenden Spieler anwaehlen */
void NextAlive()
{
        do {
                if (++PlayerCount==NumPl) PlayerCount=0;
	} while (!PlayerLive[OrderPl[PlayerCount]]);
}

/* wird waehrend des Spiels aufgerufen */
void OnGame()
{
        int flag,c;
        switch (GameState) {
	    case GAME_SET_ANGLE:
	        if (strlen(input)>2) {
	                angle=atoi(&input[2]);
	                if (input[0]=='L') angle=180-angle;
	                if (input[1]=='-') angle=-angle;
	                SetVelocity();
		} else SetAngle();
	        break;
	    case GAME_SET_VELOCITY:
	        if (strlen(input)>0) {
	                velocity=atoi(input);
			if (client) {
			        GameState=99;
			        ClearBuf('s');
				PutInt(PlayerCount);
				PutInt(angle);
				PutInt(velocity);
				MyWrite(Sock);
			} else {
		                if ((flag=PlotShot())>=0) {
				  if (CheckWinner(flag)) SetGameInfo(); else {
				    NextAlive();
				    SetAngle();
				  }
		                } else {
			                /* naechsten noch lebenden Spieler anwaehlen */
			                NextAlive();
		                        SetAngle();
				}
			}
		        Redraw();
		} else SetVelocity();
	        break;
	    case GAME_INFO:
	        if (InGame<NumGames) InitRound(); else SetGameEnd();
		if (client) {
		        ClearBuf('p');
			MyWrite(Sock);
		}
	        break;
	    case GAME_END:
	        end=1;
		ClearBuf('e');
		MyWrite(Sock);
		close(Sock);
	}
}
