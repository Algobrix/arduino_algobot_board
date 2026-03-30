/* Define to prevent recursive inclusion *********************************** */
#ifndef __MOTOR_H
#define __MOTOR_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include "pinout.h"
#include "motorEncoders.h"

/* Exported constants ****************************************************** */
/* #define debugMOTOR                                    printDEBUG                       */
#define debugMOTOR(...)  

#define NUM_OF_MOTORS 3
#define MOTOR_MAX_POWER_LEVEL 3
#define CW LOW
#define CCW HIGH


/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */
class Motor {
    private:
        byte dirPin;
        byte pwmPin;

    public:
        byte scriptRowId = 0;
        byte pwm = 0;
        boolean direction = CW;

        Motor(byte dirPin, byte pwmPin); 
        boolean isRunning();
        void start(byte pwm, boolean dir, byte scriptRowId);
        void changeSpeed(byte pwm);
        void stop(byte scriptRowId);
};
/* Exported variables ****************************************************** */
extern Motor motors[NUM_OF_MOTORS];

/* Exported functions ****************************************************** */
void motorsInit();
void myAnalogWrite(byte pin, byte val);
void prematureEndMotorScript(byte scriptRowId);


#endif 
/* ***************************** END OF FILE ******************************* */
