/* Includes **************************************************************** */
#include "led.h"
#include "comHandler.h"

/* Private constants ******************************************************* */


/* Private macros ********************************************************** */

/* Private types *********************************************************** */

/* Private variables ******************************************************* */
Led leds[NUM_OF_LEDS] = {Led(LED_A_PIN), Led(LED_B_PIN)};

/* Private function prototypes ********************************************* */

/* Exported functions ****************************************************** */
Led::Led(byte pin)
{
    neoPixelLed = Adafruit_NeoPixel(1, pin, NEO_GRB + NEO_KHZ800);
    neoPixelLed.begin();
}

void Led::start(byte r, byte g, byte b, byte scriptRowId) 
{
    // If this LED has a script row id, premature end the script row.
    if(this->scriptRowId != 0) {
        prematureEndLedScript(this->scriptRowId);
    }
    // Assign the new script row to the LED and start it.
    this->scriptRowId = scriptRowId;
    neoPixelLed.setPixelColor(0, neoPixelLed.Color(r, g, b));
    neoPixelLed.show();
}

void Led::stop(byte scriptRowId)
{
    // If the LED belongs to the the scriptRowId that called to stop.
    if(this->scriptRowId == scriptRowId) {
        this->scriptRowId = 0;
        neoPixelLed.setPixelColor(0, neoPixelLed.Color(0, 0, 0));
        neoPixelLed.show();
    }
}


void ledsInit(void) {
    for(int i = 0; i < NUM_OF_LEDS; i++) {
        leds[i].stop(leds[i].scriptRowId);
    }
}
void prematureEndLedScript(byte scriptRowId) {
    for(int i = 0; i < NUM_OF_LEDS; i++) {
        if(leds[i].scriptRowId == scriptRowId) {
            leds[i].stop(scriptRowId);
        }
    }
    // If infinity, the script row stopped and is not running anymore.
    if(scriptRowArray[scriptRowId].duration == -1) {
        numOfRunningScriptRows--;
    }
    // Else, the scriptRow duration goes down to 0 for it to finish and move to the next script row in the thread.
    else {
        scriptRowArray[scriptRowId].duration = 0;
    }
    comHandler.sendRowExecute(OP_COMMAND_ROW_EXECUTION_PREMATURE_END, scriptRowId);
}

/* Private functions ******************************************************* */

/* ***************************** END OF FILE ******************************* */
