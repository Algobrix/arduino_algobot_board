/* Define to prevent recursive inclusion *********************************** */
#ifndef __RUNTIME_H
#define __RUNTIME_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include "pinout.h"
#include "debug.h"

/* Exported constants ****************************************************** */
#define debugRUN                                    printDEBUG                      
/* #define debugRUN */                                                          

#define DURATION_MULTIPLIER 100L
#define MIN_TIME_VALUE 10L
#define MAX_TIME_VALUE 180L
 
/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */

/* Exported variables ****************************************************** */
extern boolean changeSpeed; 

/* Exported functions ****************************************************** */
void processThreads();
void processScriptRow(byte scriptRowId);
void getNextThread();
void getNextScriptRow(byte scriptRowId);
boolean processStartThread(byte scriptRowId);
boolean processStartLoop(byte scriptRowId);
boolean processEndLoop(byte scriptRowId);
boolean processMoveMotor(byte scriptRowId);
boolean processLed(byte scriptRowId);
boolean processStop(byte scriptRowId);
boolean processWaitSensor(byte scriptRowId);
boolean processWait(byte scriptRowId);
boolean processSound(byte scriptRowId);
boolean processSequence(byte scriptRowId);

void startScriptRow(byte scriptRowId);
void finishedScriptRow(byte scriptRowId);
void motorAndDirectionHelper(byte directionDataByte, byte *motorAndDirection);
byte motorPowerHelper(byte powerDataByte);
//byte motorBalanceSpeed(int stepPow, int maxPower);


#endif 
/* ***************************** END OF FILE ******************************* */
