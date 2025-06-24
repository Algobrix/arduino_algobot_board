/* Define to prevent recursive inclusion *********************************** */
#ifndef __SEQUENCES_H
#define __SEQUENCES_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include "pinout.h"
#include "debug.h"

/* Exported constants ****************************************************** */
#define debugSEQUENCES                      printDEBUG
/* #define debugSEQUENCES */

#define FORWARD 11 // A CCW - B CW
#define BACKWARDS 14 // A CW - B CCW
#define LEFT 15 // A CCW - B CCW
#define RIGHT 10 // A CW - B CW

/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */

/* Exported variables ****************************************************** */
extern boolean isStartOfLoop;
extern byte loopCounter;
extern int encA; 
extern int encB;
// parameter for garbage can
extern int playSound_11; // for garbage can
// parameters for travel random
extern byte travelRandomCase;
extern int yCurrent; // X current
extern int xCurrent; // Y current
extern int xRand; 
extern int yRand; 
extern int moveRand; 
extern int sign; 
extern int rand_dir; // 0 - Up, 2 = 90 = Right, 3 = 180 = Down, 4 = -90 = 270 = Left
extern int current_dir; // 0 - up, 2 = 90 = Right, 3 = 180 = Down, 4 = -90 = 270 = Left
//unsigned long blinkTimer ;


/* Exported functions ****************************************************** */
void nextCommand(byte scriptRowId);
void sequenceDone(byte scriptRowId);
void startLoopCommand(byte iterations);
void endLoopCommand(byte scriptRowId, byte firstCommandInLoop);
void moveMotorTime(byte scriptRowId, long duration, byte *motorAndDirection, byte pwm);

void ground_zero(boolean dir, byte scriptRowId);
void moveMotorEncoder(byte scriptRowId, long encoderDuration, byte *motorAndDirection, byte pwm);
void ledOn(byte scriptRowId, long duration, boolean *isLedSelected, byte r, byte g, byte b);

boolean garbageCanLifter(byte scriptRowId);
boolean travelRandom(byte scriptRowId);
boolean basketball(byte scriptRowId);
boolean goalKeeper(byte scriptRowId);
boolean ballKicker(byte scriptRowId);
boolean fan(byte scriptRowId);
boolean squareMovement(byte scriptRowId);
boolean ledMonkeyMove(byte scriptRowId);

#endif 
/* ***************************** END OF FILE ******************************* */
