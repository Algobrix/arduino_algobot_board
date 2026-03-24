/* Includes **************************************************************** */
#include "system.h"
#include "thread.h"
#include "led.h"
#include "motorEncoders.h"
#include "userInterface.h"
#include "soundPlayer.h"
#include "motor.h"
#include "comHandler.h"

/* Private constants ******************************************************* */


/* Private macros ********************************************************** */

/* Private types *********************************************************** */

/* Private variables ******************************************************* */
boolean isPlaying = false;
boolean isScriptLoaded = false;
boolean isPlayingFromBrain = false;
unsigned long sleepTimeoutMillis;
boolean noiseForLowBattery = false; // this will work, only on startup
/* Private function prototypes ********************************************* */

/* Exported functions ****************************************************** */
void sleepTimeoutCheck() {
  if(chk4TimeoutSYSTIM(sleepTimeoutMillis,SLEEP_TIMEOUT) == SYSTIM_TIMEOUT) {
    shutdown();
  }
}

void processPlayButton() 
{
    // Play Button Control - Act on release
    // Play Button Pressed DOWN --> digitalRead is LOW \ 0
    boolean isPlayPressed = (digitalRead(playButton.buttonPin) == LOW);
    if(playButton.buttonLastState != isPlayPressed) 
    {
        sleepTimeoutMillis = getSYSTIM(); // Reset the sleep timeout whenever a change is made.
        playButton.buttonLastState = isPlayPressed; // Save the last button state
        playButton.buttonLastStateMillis = getSYSTIM(); // Save the time of the state change
                                                        // Play button isn't pressed <--> the last state was pressed
        if(!isPlayPressed) 
        {
            // If already playing, Stop Playing.
            if(isPlaying) 
            {
                debugSYS("Stop script execution\r\n");
                stopPlaying();
            }
            // Else if not playing AND has Script to run, Start Playing.
            else if (!isPlaying && isScriptLoaded) 
            {
                isPlayingFromBrain = true;
                debugSYS("Start script execution\r\n");
                startPlaying();
            }
        }
    }

    // When playing --> the Play Button LED state is "blinking state"
    // Toggle the LED to Blink it.
    if(playButton.blinkState) 
    {
        if(chk4TimeoutSYSTIM(playButton.ledStateMillis,LED_BLINK_TIME_MILLIS) == SYSTIM_TIMEOUT) 
        {
            playButton.toggleLedState();
        }
    }  
}

void printMessage(byte *messageToProcess) 
{
  debugSYS(F("["));
  for(int i = 0; i < SCRIPT_ROW_LENGTH; i++) 
  {
    debugSYS(messageToProcess[i]);
    if(i != SCRIPT_ROW_LENGTH-1) 
    {
      debugSYS(F(", "));
    }
  }
  debugSYS(F("]\r\n"));
}

void processMessage() 
{
  byte *messageToProcess = comHandler.rxData;
  printMessage(messageToProcess);
  switch (messageToProcess[MESSAGE_BUFFER_OP_COMMAND]) 
  {
      case OP_COMMAND_START_OF_PLAY_DATA: 
      {
          isPlayingFromBrain = false;
          // Reset the Script Row Array
          for(int i = 0; i < SCRIPT_ARRAY_SIZE; i++) 
          {
              scriptRowArray[i] = ScriptRow();
          }
          currentReceivedRow = 0;
          totalRowsInData = messageToProcess[MESSAGE_BUFFER_ROW];
          comHandler.sendStartPlayData();
          debugSYS(F("Start script transmision\r\n"));
          break;
      }
      case OP_COMMAND_START_OF_PLAY_DATA_ROW: 
      {
          if (currentReceivedRow == messageToProcess[MESSAGE_BUFFER_ROW]) 
          {
              scriptRowArray[currentReceivedRow] = ScriptRow(messageToProcess[MESSAGE_BUFFER_THREAD], messageToProcess[MESSAGE_BUFFER_CUBE_TYPE], &messageToProcess[MESSAGE_BUFFER_PARAMETERS]);
              comHandler.sendRowConfirm(currentReceivedRow++);
                
              debugSYS(F("Received row: "));
              debugSYS(currentReceivedRow);
              debugSYS(F(" of the latest script\r\n"));
          } 
          else 
          {
              comHandler.sendReceiveDataError();
          }
          break;
      }
      case OP_COMMAND_END_DATA_PLAY_MODE: 
      {
          // delay(80);
          comHandler.sendEndOfPlayData();
          isScriptLoaded = true;
          isPlayingFromBrain = false;
          debugSYS(F("Script transmision completed. Start executing script\r\n"));
          startPlaying();
          break;
      }
      case OP_COMMAND_STOP: 
      {
          debugSYS(F("Stop script execution\r\n"));
          stopPlaying();
          comHandler.sendStopPlayConfirm();
          break;
      }
      case OP_COMMAND_ERROR_ON_SENT_DATA: 
      {
          debugSYS(F("There was an error on the sent data\r\n"));
          break;
      }
  }
}

void startPlaying() {
  stopPlaying(true);
  playButton.setLedBlinkState(true);
  threadArray[0].start(); // Start the "main thread"

/*  if (!noiseForLowBattery) // if battery is low, notify the user. 
  {
    int value = analogRead(POWER_METER_PIN);
    if ( value <= POWER_BLINK ) {
      debugSYS('power meter level: ');
      debugSYS(value);
      debugSYS("\r\n");
      soundPlayer.play(0, 1);
      noiseForLowBattery = true;
    }; 
  }*/


}

void stopPlaying(boolean onlyInit) {
  encodersInit();
  motorsInit();
  ledsInit();
  soundPlayer.stop();
  scriptRowsInit();
  threadsInit();
  isPlaying = false;
  playButton.setLedBlinkState(false);
  if(!onlyInit) {
    comHandler.sendEndOfPlay();
  }
}

void shutdown() {
  debugSYS(F("System Shutdown!\n Unplug USB to restart.\r\n"));
  stopPlaying();
  pinMode(playButton.buttonPin, OUTPUT);
  digitalWrite(playButton.buttonPin, LOW);
  sleepTimeoutMillis = getSYSTIM();
}

/* Private functions ******************************************************* */

/* ***************************** END OF FILE ******************************* */
