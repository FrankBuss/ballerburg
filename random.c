/*
 * 
 * GetRandom:
 * 
 * holt eine Zufallszahl innerhalb eines bestimmten positiven Bereichs
 * 
 * 
 * InitRandom:
 * 
 * initialisiert den Zufallsgenerator
 * 
*/

#include <stdlib.h>
#include <time.h>

int GetRandom(int z)
{
        int i,e;
        if (z<2) return 0;
        i=RAND_MAX/z;
        while ((e=rand()/i)>=z);
        return e;
}

int InitRandom()
{
	srand(time(NULL));
}
