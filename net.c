
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>
#include "bb.h"

extern int InpMode;
extern int angle;
extern int velocity;

/* Kommandos, die der Server auswerten kann:
 * 
 * i: Init:
 *     registriert den Clienten beim Server, schickt Kommando i als Antwort
 * r Spieleranzahl: Regist:
 *     registriert die lokalen Spieler, schickt Kommando r zurueck
 * n name1 name2 ... nameN: Names:
 *     registriert die Spielernamen
 * s Spieler Winkel Kraft: Shot:
 *     Schussdaten von Spieler
 *
 * 
 * Kommandos, die der Client auswerten kann:
 * 
 * i Runden Spieler: Init:
 *     Anzahl Runden und Anzahl Spieler
 * r flag: Regist:
 *     1 wenn ok, 0 wenn zuviel Spieler
 * n name1 name2 ... nameN: Names:
 *     Namen aller Spieler
 * s Spieler: Shot:
 *     Aufforderung, die Daten fuer den Schuss des Spielers
 *     abzufragen
 * e x1 y1 x2 y2 ... xn yn berg[1024]: nEw:
 *     neue Runde, Spielerpostionen
 *     und Berg
 * p Spieler Winkel Kraft flag berg[1024]: Paint:
 *     Anzeige eines Schusses; flag=-1, wenn kein Spieler getroffen, sonst
 *     Nummer des Spielers; Berg nach Schuss
 *
 * - Kommandos werden mit zwei Bytes fuer die Laenge des Kommandos eingeleitet
 *   (high/low), dann folgt das Kommando und die Argumente
 * - auf alle Argumente folgt ein Leerzeichen, Strings werden durch " markiert
 * - Spieler, xn, yn, flag werden als ASCII-Zeichen fuer int uebertragen
 * - berg[1024] wird als Wordfolge (high/low) uebertragen
 * - Winkel, Kraft werden als ASCII-Zeichen fuer float uebertragen
*/

extern char *ServerName;

/* nur Servervariablen */
int ClientCount;
int ServerEnd;

struct Client {
        int connected;
        int sock;
        int NumPl;
        char Name[MaxPlayer][20];
};

struct Client ClData[4];

/* andere Variablen inklusiv Servervariablen */
int Sock;
int port=0x4141;
unsigned char buf[2500];
unsigned char buf2[2500];  /* test */
char StringBuf[100];
int Index;
pid_t id;

void ClearBuf(char ch)
{
	int c;
	for (c=0;c<2499;c++) buf[c]=' ';
        Index=3;
        buf[2]=ch;
}

int WaitRead(int sock)
{
        static fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock,&fds);
	return select(sock+1,&fds,NULL,NULL,NULL);
}

void MyRead(int sock)
{
        char ch=1,ind=0;
        int st,c=2,len=0;
        while (c) {
                st=read(sock,&ch,1);
                if (st<0) {
                        if (errno!=EINTR) {
	                        fprintf(stderr,"Fehler waehrend der Uebertragung");
				perror(NULL);
	                        exit(1);
			}
		} else {
                        if (st==1) {
	   	                if (c==2) len+=((int)ch)<<8; else len+=(int)ch;
			        c--;
			}
		}
	}
        len-=2;
        Index=0;
        while (len>0) {
                st=read(sock,&buf[Index],len);
                if (st>0) {
	                Index+=st;
	                len-=st;
		} else if (errno != EINTR)  {
      	                fprintf(stderr,
			     "Fehler vor Ende aller zu empfangenen Daten");
			perror(NULL);
	                exit(1);
		}
	}
#ifdef DEBUG
        fprintf(stderr,"Bytes empfangen: %d von %d: ",Index,sock);
	if (Index>100) Index=100;
	for (len=0;len<Index;len++) buf2[len]=buf[len];
	buf2[len]=0;
	fprintf(stderr,"%s\n",buf2);
#endif
	Index=1;
}

/* auf Aktivitaet von einem Clienten warten und Socket zurueckliefern */
int WaitClient()
{
        static fd_set fds;
	int c,Max=0;
        for (c=0;c<ClientCount;c++) {
                if (ClData[c].connected) if (ClData[c].sock>Max) Max=ClData[c].sock;
	}
        Max++;
        FD_ZERO(&fds);
	for (c=0;c<ClientCount;c++) if (ClData[c].connected) FD_SET(ClData[c].sock,&fds);
	if (select(Max,&fds,NULL,NULL,NULL)>0)
	        for (c=0;c<ClientCount;c++)
	                if (FD_ISSET(ClData[c].sock,&fds)) return c;
}

void WaitAllClientAcknowledge()
{
        int c,cl;
        for (c=0;c<ClientCount;c++) {
	        MyRead(ClData[cl=WaitClient()].sock);
		if (buf[0]=='e') ClData[cl].connected=0;
	}
}

/* schreibt den Puffer in den Socket */
void MyWrite(int sock)
{
        int st,c=0,c2=Index;
        buf[Index]=0;
        buf[0]=c2>>8;
        buf[1]=c2&255;
        while (c2>0) {
                st=write(sock,&buf[c],c2);
                if (st>0) {
	                c+=st;
	                c2-=st;
		} else if (errno != EINTR) {
	                fprintf(stderr,
				"Fehler vor Uebertragung aller Daten\n");
			perror(NULL);
	                exit(1);
		}
	}
#ifdef DEBUG
	c-=2;
        fprintf(stderr,"Bytes gesendet: %d an %d:",c,sock);
	if (c>100) c=100;
	buf2[c+1]=0;
	for (;c>=0;c--) buf2[c]=buf[c+2];
	fprintf(stderr,"%s\n",buf2);
#endif
}

/* ueberliest Leerzeichen im Puffer */
void IgnoreSpace()
{
	while (buf[Index]==' '&&buf[Index]) Index++;
        if (!buf[Index]) {
                fprintf(stderr,"Stringende vor Auswertungsroutine\n");
	        exit(1);
	}
}

/* ueberliest bis Leerzeichen kommen */
void IgnoreUntilSpace()
{
	while (buf[Index]!=' '&&buf[Index]) Index++;
}

/* holt ein int aus dem buffer */
int GetInt()
{
        int c;
        IgnoreSpace();
	c=atoi(&buf[Index]);
        IgnoreUntilSpace();
	return c;
}

/* holt einen String aus dem buffer */
void GetString()
{
        int c;
	IgnoreSpace();
	if (buf[Index]=='"') {
	        c=0;
	        while (buf[++Index]!='"') StringBuf[c++]=buf[Index];
	        StringBuf[c]=0;
	} else fprintf(stderr,"Stringanfangsmarkierung nicht gefunden\n");
	Index++;
}

/* schreibt ein int als ASCII in den Puffer */
void PutInt(int i)
{
	sprintf(&buf[Index],"%d",i);
	IgnoreUntilSpace();
        buf[Index++]=' ';
}

/* schreibt einen String in den Puffer */
void PutString(char *s)
{
        buf[Index++]='"';
	while (s[0]) {
	        buf[Index++]=s[0];
	        s++;
	}
        buf[Index++]='"';
        Index++;
}

/* Berg in den Puffer schreiben */
void PutMountain()
{
        int c;
	for (c=0;c<1025;c++) {
	  	buf[c*2+Index]=(char)(Terrain[c]>>8);
		buf[c*2+Index+1]=(char)(Terrain[c]&255);
	}
	Index+=1025*2+2;
}

void GetMountain()
{
        int c;
        Index++;
	for (c=0;c<1025;c++)
	        Terrain[c]=(((int)buf[2*c+Index])<<8)+((int)buf[2*c+Index+1]);
}

static void WaitFunc(int signo)
{
        if (signo==SIGUSR1) {
	        if (WaitRead(Sock)>0) kill(id,SIGUSR1);
	}
}

/* wird fuer den Clienten gebraucht, um signalgesteuert Servermeldungen
 * auszuwerten
*/
static void SocketFunc(int signo)
{
        int c;
        if (signo==SIGUSR1) {
	        MyRead(Sock);
		Index=1;
	        switch (buf[0]) {
		    case 'i':
		        NumGames=GetInt();
	                CastleCraterRadius=GetInt();
		        ShotCraterRadius=GetInt();
		        MountainPermeability=GetInt();
			InitNumPl();
			Redraw();
		        break;
		    case 'r':
		        flag=GetInt();
		        break;
		    case 'n':
		        NumPl=GetInt();
		        for (c=0;c<NumPl;c++) {
			        GetString();
				strcpy(Name[c],StringBuf);
				OrderPl[c]=c;
			}
		        break;
		    case 's':
		        PlayerCount=GetInt();
			SetAngle();
			Redraw();
		        break;
		    case 'e':
		        for (c=0;c<NumPl;c++) {
			        CastleX[c]=GetInt();
				CastleY[c]=GetInt();
			}
			Wind=GetInt();
			GetMountain();
			InpMode=0;
		        InitRound();
		        break;
		    case 'p':
		        PlayerCount=GetInt();
			angle=GetInt();
			velocity=GetInt();
			c=GetInt();
			PlotShot();
			GetMountain();
			if (c>=0) {
			        if (CheckWinner(c)) {
				        SetGameInfo();
					Redraw();
					break;
				}
			}
			Redraw();
		        ClearBuf('p');
		        MyWrite(Sock);
		        break;
		    default:
		        fprintf(stderr,"Kommando nicht erkannt: %s\n",buf);
		}
        }
	kill(id,SIGUSR1);
}

void ClientInit()
{
        struct hostent *host;
	struct sigaction act;
        struct sockaddr_in sa_client;
	act.sa_flags=0;
	sigemptyset(&act.sa_mask);
        if ((Sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
                fprintf(stderr,"Konnte Socket nicht erstellen.");
		perror(NULL);
	        exit(1);
	}
        host=gethostbyname(ServerName);
        bzero((void *)&sa_client, sizeof sa_client);
        sa_client.sin_family=AF_INET;
        memcpy(&sa_client.sin_addr, host->h_addr, host->h_length);
        sa_client.sin_port=htons(port);
        if (connect(Sock,(struct sockaddr *) &sa_client,sizeof sa_client)) {
                fprintf(stderr,"Verbinungsaufbau zum Server schlug fehl\n");
		perror(NULL);
	        exit(1);
        }
        id=fork();
        if (!id) {
	        id=getppid();
		act.sa_handler=(void (*)(int))WaitFunc;
	        if (sigaction(SIGUSR1,&act,NULL)) {
	                fprintf(stderr,"konnte SIGUSR1 nicht holen\n");
	                exit(1);
		}
		WaitFunc(SIGUSR1);
                while (1) wait();
	}
	sleep(1);
        act.sa_handler=(void (*)(int))SocketFunc;
	if (sigaction(SIGUSR1,&act,NULL)) {
	        fprintf(stderr,"konnte SIGUSR1 nicht holen\n");
	        exit(1);
	}
}

void ServerInit(int clCount)
{
        struct sockaddr_in sa_server;
        struct sockaddr_in sa_client;
        int c,c2;
        int v=1;
        if ((Sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0) {
	        fprintf(stderr,"Konnte Socket nicht erstellen.\n");
		perror(NULL);
	        exit(1);
	}
        /* Socket soll nach beenden sofort wieder freigegeben werden */
        setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR,(void *)&v, sizeof(v));
        bzero((void *)&sa_server, sizeof sa_server);
	sa_server.sin_family=AF_INET;
	sa_server.sin_addr.s_addr=htonl(INADDR_ANY);
	sa_server.sin_port=htons(port);
        if(bind(Sock,(struct sockaddr *) &sa_server,sizeof sa_server)<0) {
	        fprintf(stderr,"konnte nicht Port %d benutzen",port);
		perror(NULL);
		exit(1);
	}
        if (listen(Sock,5)<0) {
                fprintf(stderr,"Befehl listen lieferte Fehler");
		perror(NULL);
	        exit(1);
        }
        c2=0;
        ClientCount=clCount;
        while (c2<clCount)  {
                c=sizeof(sa_client);
                ClData[c2].sock=accept(Sock, (struct sockaddr *)&sa_client, &c);
		ClData[c2].NumPl=0;
                if (ClData[c2].sock<0) {
		        fprintf(stderr,"Befehl accept lieferte Fehler");
			perror(NULL);
			ClData[c2].connected=0;
                } else {
		        ClData[c2].connected=1;
		        c2++;
		}
	}
}

/* Aufforderung des Servers an aktuellen Spieler, Winkel und Kraft zu senden */
void NextPlayer()
{
        int cl=-1,c;
        ClearBuf('s');
	PutInt(c=OrderPl[PlayerCount]);
	while (c+1>ClData[++cl].NumPl) c-=ClData[cl].NumPl;
      	MyWrite(ClData[cl].sock);
}

void NextRound()
{
        int c;
	if (++InGame>NumGames) {
	        ServerEnd=1;
		return;
	}
        MakeBattleField();
	PlaceCastles();
        for (c=0;c<NumPl;c++) PlayerLive[c]=1;  /* aktuelles Lebensflag */
	ClearBuf('e');
	for (c=0;c<NumPl;c++) {
	        PutInt(CastleX[c]);
		PutInt(CastleY[c]);
	}
	PutInt(Wind);
	PutMountain();
	for (c=0;c<ClientCount;c++) MyWrite(ClData[c].sock);
	PlayerCount=0;
	NextPlayer();
}

void ServerEvent(int c)
{
        int t,co,pos,flag;
	MyRead(ClData[c].sock);
	Index=1;
        switch (buf[0]) {
	    case 'i':  /* Client wird registriert */
	        NumPl=0;
	        ClearBuf('i');
	        PutInt(NumGames);
	        PutInt(CastleCraterRadius);
		PutInt(ShotCraterRadius);
		PutInt(MountainPermeability);
	        MyWrite(ClData[c].sock);
                break;
            case 'r':  /* registriert Anzahl lokaler Spieler */
	        ClData[c].NumPl=GetInt();
		co=0;
	        for (t=0;t<ClientCount;t++)
		        if (ClData[t].NumPl) co+=ClData[t].NumPl; else co++;
	        ClearBuf('r');
		if (co>MaxPlayer) PutInt(0); else {
		        PutInt(1);
			NumPl+=ClData[c].NumPl;
		}
		MyWrite(ClData[c].sock);
	        break;
	    case 'n':
		for (t=0;t<ClData[c].NumPl;t++) {
		        GetString();
			strcpy(ClData[c].Name[t],StringBuf);
		}
	        for (t=0;t<ClientCount;t++) if (!ClData[t].NumPl) return;
		/* wenn alle registriert sind, dann Namen uebetragen
		 * und Spiel beginnen */
		pos=0;
		for (co=0;co<ClientCount;co++) {
		        for (t=0;t<ClData[co].NumPl;t++)
			        strcpy(Name[pos++],ClData[co].Name[t]);
		}
	        ClearBuf('n');
		PutInt(NumPl);
	        for (t=0;t<NumPl;t++) PutString(Name[t]);
		for (t=0;t<ClientCount;t++) MyWrite(ClData[t].sock);
		NextRound();
                break;
            case 's':
	        /* Schuss registrieren */
	        t=GetInt();  /* Spielernummer */
	        angle=GetInt();
		velocity=GetInt();
		/* an alle weitersenden */
		ClearBuf('p');
		PutInt(t);
		PutInt(angle);
		PutInt(velocity);
		flag=PlotShot();
		PutInt(flag);
		PutMountain();
	        for (t=0;t<ClientCount;t++) MyWrite(ClData[t].sock);
		WaitAllClientAcknowledge();
		NextAlive();
		if (CheckWinner(flag)) NextRound(); else NextPlayer();
	        break;
	    default:
                fprintf(stderr,"Kommando nicht erkannt: %s\n",buf);
        }
}

void ServerLoop()
{
	ServerEnd=0;
        while (!ServerEnd) ServerEvent(WaitClient());
	WaitAllClientAcknowledge();
	sleep(1);
  /* auf close bei allen Clienten warten */
}
