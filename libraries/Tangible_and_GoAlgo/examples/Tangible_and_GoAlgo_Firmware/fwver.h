/**
* @file        fwver.h
* @author      semir-t
* @date        Januar 2024
* @version     1.0.0
*/

/* Define to prevent recursive inclusion *********************************** */
#ifndef __FWVER_H
#define __FWVER_H
/* Includes **************************************************************** */

/* Module configuration **************************************************** */

/* Exported constants ****************************************************** */

/* Exported macros ********************************************************* */
#define FIRMWARE_VERSION_MAJOR        3
#define FIRMWARE_VERSION_MINOR        21
#define FIRMWARE_VERSION_STRING       "3.21"
#define FIRMWARE_VERSION              ((uint8_t)(((FIRMWARE_VERSION_MAJOR & 0x0f) << 4) | (FIRMWARE_VERSION_MINOR & 0x0f)))
/* Exported types ********************************************************** */

/* Exported variables ****************************************************** */

/* Exported functions ****************************************************** */

#endif 
