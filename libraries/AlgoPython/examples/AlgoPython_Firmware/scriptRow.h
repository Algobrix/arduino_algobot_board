/* Define to prevent recursive inclusion *********************************** */
#ifndef __SCRIPTROW_H
#define __SCRIPTROW_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include "pinout.h"

/* Exported constants ****************************************************** */
#define SCRIPT_ARRAY_SIZE 30
#define DATA_BYTES_SIZE 8

const typedef enum {
  TYPE_START_THREAD = 1,
  TYPE_MOVE_MOTOR = 2,
  TYPE_WAIT_SENSOR = 3,
  TYPE_WAIT = 4,
  TYPE_SOUND = 5,
  TYPE_START_LOOP = 6,
  TYPE_END_LOOP = 7,
  TYPE_CALC = 8,
  TYPE_LED = 9,
  TYPE_COMBINE_THREAD = 10,
  TYPE_STOP = 11,
  TYPE_PROGRAM = 12
} scriptRowType;
/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */
class ScriptRow {
  public:
  byte threadId = 0;
  byte type = 0;
  byte dataBytes[DATA_BYTES_SIZE] = {};
  byte counter = 0;
  long duration = 0;
  long startTime = -1;
  uint16_t encoder_cnt = 0;
  bool isRunning = false;

  ScriptRow() {};
  ScriptRow(byte threadId, byte type, byte *dataBytes); 
  void reset();
};

/* Exported variables ****************************************************** */
extern byte totalRowsInData;         // Total Rows of data Expected
extern byte currentReceivedRow;      // Current Script Row Received
extern byte numOfRunningScriptRows;

extern ScriptRow scriptRowArray[SCRIPT_ARRAY_SIZE];
/* Exported functions ****************************************************** */
void scriptRowsInit();
#endif 
/* ***************************** END OF FILE ******************************* */
