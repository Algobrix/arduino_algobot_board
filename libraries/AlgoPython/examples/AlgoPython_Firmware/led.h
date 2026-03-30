/* Define to prevent recursive inclusion *********************************** */
#ifndef __LED_H
#define __LED_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include "pinout.h"
#include "scriptRow.h"
#include <Adafruit_NeoPixel.h>

/* Exported constants ****************************************************** */
#define NUM_OF_LEDS 2

/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */
class Led {
    private:
        Adafruit_NeoPixel neoPixelLed;

    public:
        byte scriptRowId = 0;
        Led(byte pin);
        void start(byte r, byte g, byte b, byte scriptRowId);
        void stop(byte scriptRowId);
};



/* Exported variables ****************************************************** */
extern Led leds[NUM_OF_LEDS];

/* Exported functions ****************************************************** */
void ledsInit(void);
void prematureEndLedScript(byte scriptRowId);

#endif 
/* ***************************** END OF FILE ******************************* */
