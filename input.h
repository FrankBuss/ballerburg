/*
 * fuer die Eingaberoutine
*/

#define NO_INP 0       /* es ist keine Dialogbox angezeigt */
#define INFO_INP 1     /* wartet auf beliebigen Tastendruck, InpLength egal */
#define JA_NEIN_INP 2  /* wartet auf jJ/nN, InpLength egal */
#define NUMBER_INP 3   /* laesst 0..9 zu */
#define ANGLE_INP 4    /* laesst lL, rR, +, - und 0..9 zu */
#define TEXT_INP 5     /* Buchstaben, Zeichen und Zahlen */

#define MAX_INP_LENGTH 256

extern char question[MAX_INP_LENGTH];
extern int InpMode;  /* kann die Werte der Defines oben annehmen */
extern int InpLength;  /* maximale Zeichen fuer aktuelle Frage */
extern char input[MAX_INP_LENGTH];
extern unsigned long InpBoxPixel;
extern unsigned long InpLightPixel;
extern unsigned long InpDarkPixel;

