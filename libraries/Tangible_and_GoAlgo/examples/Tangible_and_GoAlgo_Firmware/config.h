/**
 * @file        config.h
 * @author      semir-t
 * @date        December 2024
 * @version     1.0.0
 */

/* Define to prevent recursive inclusion *********************************** */
#ifndef __CONFIG_H
#define __CONFIG_H
/* Includes **************************************************************** */

/* Module configuration **************************************************** */

/* Exported constants ****************************************************** */

/*
 * Levels discrete ranges
 * +-------+------------------+
 * | Level | Range (cm)       |
 * +-------+------------------+
 * |   1   |  2.0  -   6.5   |
 * |   2   |  6.6  -  13.0   |
 * |   3   | 13.1  -  19.5   |
 * |   4   | 19.6  -  26.0   |
 * |   5   | 26.1  -  32.5   |
 * |   6   | 32.6  -  39.0   |
 * |   7   | 39.1  -  45.5   |
 * |   8   | 45.6  -  52.0   |
 * |   9   | 52.1  -  58.5   |
 * +-------+------------------+
 */


#define USE_CUSTOM_LEVEL_LIMITS             1
#define CUSTOM_LEVEL1_MIN                   1   
#define CUSTOM_LEVEL1_MAX                   1   
#define CUSTOM_LEVEL2_MIN                   1   
#define CUSTOM_LEVEL2_MAX                   3   
#define CUSTOM_LEVEL3_MIN                   1   
#define CUSTOM_LEVEL3_MAX                   4   
/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */

/* Exported variables ****************************************************** */

/* Exported functions ****************************************************** */

#endif 
