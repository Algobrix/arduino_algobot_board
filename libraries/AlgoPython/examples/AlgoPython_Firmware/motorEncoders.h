/* Define to prevent recursive inclusion *********************************** */
#ifndef __MOTORENCODERS_H
#define __MOTORENCODERS_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include "pinout.h"
#include "scriptRow.h"
#include "debug.h"

/* Exported constants ****************************************************** */
#define debugENCODER                                    printDEBUG                      
/* #define debugENCODER */                                                          

/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */

/* Exported variables ****************************************************** */
extern byte scriptRowIdEncoderA;
extern byte scriptRowIdEncoderB;
extern byte scriptRowIdEncoderC;

//int mapStep = 1555; // snail transmission 3 PPR motor
//int tenDegrees = 157; // sets the rotations 3 PPR motor
extern int mapStep; // snail transmission 6 PPR motor
//int mapStep = 4550; // snail transmission 6 PPR motor 20 cm
extern int tenDegrees; // sets the rotations 6 PPR motor
extern int diff;
extern int preDiff; // parameter for PD control

extern byte tempPowerA; 
extern byte tempPowerB;
extern int maxPower; 
extern int minPower; 
extern int stepPow; // power step size
extern unsigned long motorBalanceTimer;
extern int encoderDuration; // Maximum by default
// For custom PWM of motors A and B :  
extern boolean isHighA;
extern boolean isHighB;

/* Exported functions ****************************************************** */
void encodersInit(); 
void initOneEncoder(int motorId); 
void setMotorInterrupt(byte motorId, long timeToInterruprt, byte scriptRowId);
void checkEncodersOfMotor(byte motorId);
void myAnalogWrite(byte pin, byte val);

boolean motorBalanceSpeed();

#endif 
/* ***************************** END OF FILE ******************************* */
