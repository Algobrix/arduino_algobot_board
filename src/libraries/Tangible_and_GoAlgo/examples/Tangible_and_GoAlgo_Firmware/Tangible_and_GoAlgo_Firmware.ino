/*
 * Company:
 *  Algobrix
 * 
 * Naming Conventions :
 * https://google.github.io/styleguide/cppguide.html
 * Following the above as much as possible
 */ 

#include "main.h"
#include "fwver.h"

void setup() 
{
    initDEBUG(115200);
    printDEBUG("Algobrain Software ");
    printDEBUG((FIRMWARE_VERSION >> 4) & 0x0f);
    printDEBUG(".");
    printDEBUG((FIRMWARE_VERSION) & 0x0f);
    printDEBUG("\r\n");
    sleepTimeoutMillis = getSYSTIM();
    randomSeed(getSYSTIM());
    stopPlaying(true);
}

void loop() 
{
  processPlayButton();
 
  comHandler.processIncomingData();
  if(comHandler.isMessageWaitingForProcess) 
  {
    processMessage();
    comHandler.isMessageWaitingForProcess = false;
    comHandler.clearSerialBuffer();
  }
  
  if(isPlaying) 
  {
    processThreads();
  } 
  else 
  {
    sleepTimeoutCheck();
    powerManager.checkPower();
  }
  
}
