/*
 * Routinen fuer die Zustandsverwaltung, z.B. OnInit, OnGame
*/

#define INIT_NUM_PL 1
#define INIT_NAMES 2
#define INIT_NUM_GAMES 3

#define GAME_SET_ANGLE 1
#define GAME_SET_VELOCITY 2
#define GAME_INFO 3
#define GAME_END 4

extern void OnInit();
extern void OnGame();
extern void InitRound();
extern void InitNumPl();

