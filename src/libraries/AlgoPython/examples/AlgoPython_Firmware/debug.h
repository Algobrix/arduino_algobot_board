/* Define to prevent recursive inclusion *********************************** */
#ifndef __DEBUG_H
#define __DEBUG_H

/* Includes **************************************************************** */
#include <Arduino.h>

/* Exported constants ****************************************************** */

/* Exported macros ********************************************************* */
#define SYSTEM_DEBUG
#ifdef SYSTEM_DEBUG
#define initDEBUG(baudrate)             Serial.begin(baudrate)
#define printDEBUG                      Serial.print
#else
#define initDEBUG(baudrate)             
#define printDEBUG                      
#endif

/* Exported types ********************************************************** */

/* Exported variables ****************************************************** */

/* Exported functions ****************************************************** */

#endif 
/* ***************************** END OF FILE ******************************* */
