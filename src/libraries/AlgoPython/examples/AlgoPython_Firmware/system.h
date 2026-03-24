/* Define to prevent recursive inclusion *********************************** */
#ifndef __SYSTEM_H
#define __SYSTEM_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include "pinout.h"
#include "systim.h"
#include "debug.h"

/* Exported constants ****************************************************** */
#define debugSYS                                    printDEBUG                      
/* #define debugSYS */                                                          
/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */

/* Exported variables ****************************************************** */
extern boolean isPlaying;
extern boolean isScriptLoaded;
extern boolean isPlayingFromBrain;
extern unsigned long sleepTimeoutMillis;
extern boolean noiseForLowBattery; 

/* Exported functions ****************************************************** */
void sleepTimeoutCheck();
void processPlayButton();
void processMessage();
void startPlaying();
void stopPlaying(boolean onlyInit=false);
void shutdown();
#endif 
/* ***************************** END OF FILE ******************************* */
