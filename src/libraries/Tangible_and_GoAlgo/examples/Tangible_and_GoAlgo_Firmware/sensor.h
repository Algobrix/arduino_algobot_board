/*
  * Documentation on the Distance Sensor:
  * Senses distance between 2 - 100 centimeters
  * The 100 cm is divided between 0-10 in the code
  * 0-2 cm will match sensor output of 0 --> Sense below 2 cm --> False
  * 2-20 .......................... of 1-2 --> Low / True
  * 20-50 ......................... of 3-5 --> Med / True
  * 50-100 ........................ of 5-10 --> High / True
  * 100+ ...........................of 0 --> Sense above 100 cm --> False
*/
/* Define to prevent recursive inclusion *********************************** */
#ifndef __SENSOR_H
#define __SENSOR_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include "pinout.h"
#include "debug.h"
#include "owi.h"

/* Exported constants ****************************************************** */
#define debugSENSOR                             printDEBUG
/* #define debugSENSOR */
#define NUM_OF_SENSORS 2
//#define PULSE_TIMEOUT 4000ul // ul = unsigned long
#define PULSE_TIMEOUT 10000ul // ul = unsigned long
#define CYCLE_TIME 2000.0f // 2 MS @ Microseconds 500Hz Frequency // f = float
                           //
enum ALGOSENSOR_TYPE
{
    ALGOSENSOR_TYPE_PWM = 0x00,
    ALGOSENSOR_TYPE_1WIRE,
};
/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */

class SoftwareUART {
public:
    SoftwareUART(uint8_t rxPin);
    int rx_byte(uint8_t* value, unsigned long timeout);
    void tx_byte(uint8_t value);
    void write(uint8_t* data, size_t size);

private:
    uint8_t rxPin;
    static const unsigned int bitPeriod = 104; // Microseconds for 9600 baud
};


class Sensor {
    private:
        byte pin;
        uint8_t type;
        SoftwareUART serial;
    public:
        Sensor(uint8_t pin);
        uint8_t getValue(uint8_t *sensor_type);
        void cmd_tx(uint8_t cmd,uint8_t * payload,uint8_t size);
        int8_t cmd_get_response(uint8_t * cmd,uint8_t * payload, uint8_t * size);
};

/* Exported variables ****************************************************** */
extern Sensor sensors[NUM_OF_SENSORS];

/* Exported functions ****************************************************** */

#endif 
/* ***************************** END OF FILE ******************************* */
