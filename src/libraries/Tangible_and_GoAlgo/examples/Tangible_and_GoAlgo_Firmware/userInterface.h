/* Define to prevent recursive inclusion *********************************** */
#ifndef __USERINTERFACE_H
#define __USERINTERFACE_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include "pinout.h"
#include "debug.h"

/* Exported constants ****************************************************** */
/* #define debugUI                                    printDEBUG                       */
#define debugUI(...)
/* #define debugUI */                                                          

#define LED_BLINK_TIME_MILLIS 500
#define POWER_CHECK_EVERY_X_MILLIS 2500
#define FIRMWARE_VERSION_EVERY_X_MILLIS 1250
#define POWER_SOUND_ALERT_MILLIS 600000 // Every 10 min
//#define POWER_MAX 900	// Full Power - Currently not used, since we don't have Green light. 
#define POWER_HALF 750 	// Half Power - Red LED Blink, indicates battery will soon be finished
#define POWER_BLINK 700 	// Empty Power - Red LED Blink
//#define SLEEP_TIMEOUT 600000 // 10 Minutes = 600000 Milliseconds
#define SLEEP_TIMEOUT 2400000 // 40 Minutes = 600000 Milliseconds
#define SHUT_DOWN_MILLIS 1500 // 1.5 Seconds delay for a manual shutdown

#define R1 20000.0 // resistance of R1 (20K)
#define R2 10000.0 // resistance of R2 (10K)


/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */
class Button {
    public:
        byte buttonPin;
        byte ledPin;
        boolean buttonState;
        boolean buttonLastState;
        unsigned long buttonLastStateMillis;
        boolean ledState;
        boolean blinkState;
        unsigned long ledStateMillis;

        Button(uint8_t buttonPin, uint8_t ledPin);
        void setLedState(boolean newState);
        void toggleLedState();
        void setLedBlinkState(boolean newState);

};

class PowerManger {
  private:
  uint8_t powerLedPin = 0;
  uint8_t powerMeterPin = 0;
  unsigned long lastPowerCheckMillis = 0;
  uint8_t sendFirmwareVersionFlag = 0;
  unsigned long soundPowerCheckMillis = 0;
  boolean isPowerLedBlink = false;
  boolean isFirstSoundCheck = true;
  boolean isFirstPowerLedOn = true; 
  unsigned long lastPowerLedChangeMillis = 0;
  boolean powerLedState = false;
  int batteryLevel = 0;

  public:
  // Constructor
  PowerManger(uint8_t powerLedPin ,uint8_t powerMeterPin);
  void checkPower();
  int getBattery();
};

/* Exported variables ****************************************************** */
extern Button playButton;
extern PowerManger powerManager;
/* Exported functions ****************************************************** */

#endif 
/* ***************************** END OF FILE ******************************* */
