/* Includes **************************************************************** */
#include "scriptRow.h"

/* Private constants ******************************************************* */


/* Private macros ********************************************************** */

/* Private types *********************************************************** */

/* Private variables ******************************************************* */
byte totalRowsInData = 0;         // Total Rows of data Expected
byte currentReceivedRow = 0;      // Current Script Row Received
byte numOfRunningScriptRows = 0;

ScriptRow scriptRowArray[SCRIPT_ARRAY_SIZE];
/* Private function prototypes ********************************************* */

/* Exported functions ****************************************************** */
ScriptRow::ScriptRow(byte threadId, byte type, byte *dataBytes) {
    this->threadId = threadId;
    this->type = type;
    for(int i = 0; i < DATA_BYTES_SIZE; i++) {
        this->dataBytes[i] = dataBytes[i];
    }
}
void ScriptRow::reset() {
    this->duration = 0;
    this->counter = 0;
    this->startTime = -1;
}


void scriptRowsInit() {
  for (int i = 0; i < totalRowsInData; i++) {
    scriptRowArray[i].reset();
  }
  numOfRunningScriptRows = 0;
}

/* Private functions ******************************************************* */

/* ***************************** END OF FILE ******************************* */
