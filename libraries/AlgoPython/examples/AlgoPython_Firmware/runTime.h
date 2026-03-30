/* Define to prevent recursive inclusion *********************************** */
#ifndef __RUNTIME_H
#define __RUNTIME_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include "pinout.h"
#include "debug.h"

/* Exported constants ****************************************************** */
#define debugRUN                                    printDEBUG                      
/* #define debugRUN */                                                          

#define DURATION_MULTIPLIER 100L
#define MIN_TIME_VALUE 10L
#define MAX_TIME_VALUE 180L
 
/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */

/* Exported variables ****************************************************** */
extern boolean changeSpeed; 

/* Exported functions ****************************************************** */
void processThreads();
void processScriptRow(byte scriptRowId);
void getNextThread();
void getNextScriptRow(byte scriptRowId);
boolean processStartThread(byte scriptRowId);
boolean processStartLoop(byte scriptRowId);
boolean processEndLoop(byte scriptRowId);

/*AlgoPython Functions ------------------------------------------------------*/
void algopython_fsm_update();
uint8_t cmd_process_light(uint8_t led_port, uint8_t led_type,float led_duration, uint8_t led_power, uint8_t led_r, uint8_t led_g, uint8_t led_b);
uint8_t cmd_process_move(uint8_t motor_port,uint8_t motor_type,float motor_duration ,uint8_t motor_power,uint8_t motor_dir); 
uint8_t cmd_process_playSound(uint8_t sound_id, uint8_t volume);
uint8_t cmd_process_move_stop(uint8_t motor_port);
uint8_t cmd_process_light_stop(uint8_t light_port);
uint8_t cmd_process_sound_stop();
uint8_t cmd_process_wait_sensor(uint8_t sensor_port, uint8_t min, uint8_t max);


void fillStatusPayload(uint8_t* buf);
void startScriptRow(byte scriptRowId);
void finishedScriptRow(byte scriptRowId);
void ALGOPYTHON_motorAndDirectionHelper(byte directionDataByte, byte *motorAndDirection);
void motorAndDirectionHelper(byte directionDataByte, byte *motorAndDirection);
byte motorPowerHelper(byte powerDataByte);
//byte motorBalanceSpeed(int stepPow, int maxPower);


#endif 
/* ***************************** END OF FILE ******************************* */
