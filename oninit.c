/*
 * wird waehrend des Init-Vorgangs abgearbeitet
*/

#include <stdio.h>
#include "bb.h"
#include "state.h"
#include "input.h"

int tmpNumPl;

void InitNumPl()
{
        int c;
        for (c=0;c<MaxPlayer;c++) PlayerWin[c]=0;
        InitGame=INIT_NUM_PL;
	InpLength=1;
	input[0]=0;
	if (client) strcpy(question,"Anzahl lokaler Spieler (1..4): "); else
	  strcpy(question,"Anzahl Spieler (2..4): ");
	InpMode=NUMBER_INP;
	DrawInpBox();
}

void InitPlayerName(int z)
{
	strcpy(question, "Spieler X: ");
	question[8]=z+'1';
	InpMode=TEXT_INP;
	InpLength=10;
	input[0]=0;
	DrawInpBox();
}

void InitNumGames()
{
        int c;
        if (client) {
	        InpMode=InitGame=0;
	        ClearBuf('n');
                for (c=0;c<NumPl;c++) {
		        PlayerWin[c]=0;
			PutString(Name[c]);
		}
		MyWrite(Sock);
	} else {
                InitGame=INIT_NUM_GAMES;
	   	InpLength=3;
        	input[0]=0;
	        strcpy(question,"Wieviel Spiele: ");
	        InpMode=NUMBER_INP;
	        DrawInpBox();
	}
}

/* wird waehrend des Init-Prozesses aufgerufen, wenn InpMode==0 */
void OnInit()
{
        int c,fl;
        char *cp;
        switch (InitGame)   {
            case INIT_NUM_PL:
                if (input[0]>='2'-client&&input[0]<='4')  {
                        NumPl=input[0]-'0';
                        flag=1;
			if (client) {
                                ClearBuf('r');
			        PutInt(NumPl);
			        flag=0;
			        MyWrite(Sock);
				pause();
			}
			if (flag) {
		                tmpNumPl=0;
				input[0]=0;
				InitPlayerName(0);
				InitGame=INIT_NAMES;
			} else InitNumPl();
                } else InitNumPl();
                break;
            case INIT_NAMES:
	        /* Leerzeichen am Ende killen */
                c=strlen(input);
	        while (c>0) if (input[--c]==' ') input[c]=0; else break;
                /* wenn was geschrieben wurde, naechster Name, sonst wiederholen */
	        if (strlen(input)>0)  {
                        /* Leerzeichen am Anfang killen */
                        cp=input;
		        while (cp[0]==' ') cp++;
		        /* gibt es den Namen schon? */
		        fl=1;
		        for (c=0;c<tmpNumPl;c++) {
			        if (strcmp(cp,Name[c])==0) {
			                fl=0;
			                break;
		                }
		        }
		        if (fl) {
		                strcpy(Name[tmpNumPl],cp);
	                        if (++tmpNumPl<NumPl) InitPlayerName(tmpNumPl); else InitNumGames();
			} else InitPlayerName(tmpNumPl);
	        } else InitPlayerName(tmpNumPl);
                break;
            case INIT_NUM_GAMES:
	        if (!client) {
		        NumGames=atoi(input);
                        if (NumGames>0) {
                                for (c=0;c<NumPl;c++) PlayerWin[c]=0;
		                InitRound();
		         } else InitNumGames();
		}
                break;
	}
}

