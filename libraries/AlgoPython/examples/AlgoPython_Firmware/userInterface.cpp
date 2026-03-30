/* Includes **************************************************************** */
#include "userInterface.h"
#include "systim.h"
#include "soundPlayer.h"
#include "comHandler.h"
#include "fwver.h"

/* Private constants ******************************************************* */


/* Private macros ********************************************************** */

/* Private types *********************************************************** */

/* Private variables ******************************************************* */
Button playButton(PLAY_BUTTON_PIN, PLAY_LED_PIN);
PowerManger powerManager(POWER_LED_PIN, POWER_METER_PIN);

/* Private function prototypes ********************************************* */

/* Exported functions ****************************************************** */
Button::Button(uint8_t buttonPin, uint8_t ledPin) {
    this->buttonPin = buttonPin;
    this->ledPin = ledPin;
    buttonLastState = true;
    buttonLastStateMillis = getSYSTIM();
    buttonState = true;
    ledState = false;
    ledStateMillis = getSYSTIM();
    blinkState = false;

    pinMode(this->buttonPin, INPUT);
    pinMode(this->ledPin, OUTPUT);
}
void Button::setLedState(boolean newState) {
    ledStateMillis = getSYSTIM();
    ledState = newState;
    digitalWrite(ledPin, newState);
}
void Button::toggleLedState() {
    setLedState(!ledState);
}
void Button::setLedBlinkState(boolean newState) {
    blinkState = newState;
    setLedState(newState);
}

PowerManger::PowerManger(uint8_t powerLedPin ,uint8_t powerMeterPin) {

    // Power Led & Power Check Pins Definitions
    this->powerLedPin = powerLedPin;
    this->powerMeterPin = powerMeterPin;   
    // Turn Power LED ON
    //pinMode(powerLedPin, OUTPUT);
    //digitalWrite(powerLedPin, HIGH);
    // Setting the LED to off as a basic state. 
    if (isFirstPowerLedOn) {
        pinMode(powerLedPin, OUTPUT);
        digitalWrite(powerLedPin, LOW);
        isFirstPowerLedOn = false; 
    }
    
  }
  void PowerManger::checkPower() {
    // Power LED Blinking State
    if (isPowerLedBlink) {
      // Every 350 Millis
      // if (lastPowerLedChangeMillis + 350 < getSYSTIM()) {
      if (chk4TimeoutSYSTIM(lastPowerLedChangeMillis,350) == SYSTIM_TIMEOUT) {
      // State changes
      powerLedState = !powerLedState;
      lastPowerLedChangeMillis = getSYSTIM();
      // Set the LED to the opposite state
      digitalWrite(powerLedPin, powerLedState);
      }
    }
  

    // Read every 10 millis
    if ((getSYSTIM()) % 10 == 0)  {
      batteryLevel = 0.9 * batteryLevel + 0.1 * analogRead(powerMeterPin);
    }
    
    // if (lastPowerCheckMillis + POWER_CHECK_EVERY_X_MILLIS < getSYSTIM()) {
    if (chk4TimeoutSYSTIM(lastPowerCheckMillis,POWER_CHECK_EVERY_X_MILLIS) == SYSTIM_TIMEOUT) 
	{
		sendFirmwareVersionFlag = 0;
		int value = getBattery();
		comHandler.sendBatteryLevel(value);
		// Setting the LED
		if (value <= POWER_BLINK) {
			isPowerLedBlink = true;
			digitalWrite(powerLedPin, HIGH);      

		} else {
			isPowerLedBlink = false;
			//digitalWrite(powerLedPin, HIGH);
			if (value <= POWER_HALF)  {
				digitalWrite(powerLedPin, HIGH);
				// if (soundPowerCheckMillis + POWER_SOUND_ALERT_MILLIS < getSYSTIM() || (isFirstSoundCheck))        { // check every POWER_SOUND_ALERT_MILLIS
				if ((chk4TimeoutSYSTIM(soundPowerCheckMillis,POWER_SOUND_ALERT_MILLIS) == SYSTIM_TIMEOUT) || (isFirstSoundCheck))        
                { // check every POWER_SOUND_ALERT_MILLIS
					// soundPlayer.play(0, 1); // 0 is sound number zero, and 1 is a row ID to run.
					soundPowerCheckMillis = getSYSTIM(); 
					isFirstSoundCheck = false; 
				}
			}
				else{
					digitalWrite(powerLedPin, LOW); // Don't light up in RED as no indication is needed. 
				}            
			}
			// Set last check
			lastPowerCheckMillis = getSYSTIM();
		}
		else if (chk4TimeoutSYSTIM(lastPowerCheckMillis,FIRMWARE_VERSION_EVERY_X_MILLIS / 2) == SYSTIM_TIMEOUT) 
		{
			if(sendFirmwareVersionFlag == 0)
			{
				sendFirmwareVersionFlag = 1;
				comHandler.sendFirmwareVersion(FIRMWARE_VERSION);
			}
		}


  }
  int PowerManger::getBattery() 
  {
    float vout = 0.0;
    float vin = 0.0;
    vout = (batteryLevel * 5.0) / 1024.0;
    vin = vout / (R2 / (R1 + R2));
    int value = vin * 100; // Turn to int from float
    if ((int)vin == 0) 
    {
        debugUI(F("USB Power Source. Battery voltage: " + (String)vin + "V\r\n"));
    } 
    else 
    {
        debugUI(F("Battery voltage: " + (String)vin + "V\r\n"));
    }
    return value;
  }

/* Private functions ******************************************************* */

/* ***************************** END OF FILE ******************************* */
