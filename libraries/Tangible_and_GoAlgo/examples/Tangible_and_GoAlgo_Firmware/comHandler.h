/* Define to prevent recursive inclusion *********************************** */
#ifndef __COMHANDLER_H
#define __COMHANDLER_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include <Crc16.h>
#include "pinout.h"
#include "scriptRow.h"
#include "debug.h"

/* Exported constants ****************************************************** */
/* #define debugCOM                                    printDEBUG */
#define debugCOM(...)

/* #define debugCOM */                                    

// Bluetooth Low Energy Serial :
#define BLE_UART Serial1
#define BLE_BAUDRATE 115200

// Communication Protocol
#define SCRIPT_ROW_LENGTH 14
#define SERIAL_TIME_OUT_MILLIS 1000
#define START_OF_MESSAGE_SYMBOL 255
#define END_OF_MESSAGE_SYMBOL_1 254
#define END_OF_MESSAGE_SYMBOL_2 253
#define END_OF_MESSAGE_SYMBOL_3 252
#define TOTAL_BYTES_OP_COMMAND_NO_CRC 2

// Operational Commands Receiving From Play
#define OP_COMMAND_START_OF_PLAY_DATA 100
#define OP_COMMAND_START_OF_PLAY_DATA_ROW 101
#define OP_COMMAND_END_DATA_PLAY_MODE 103
#define OP_COMMAND_STOP 110
#define OP_COMMAND_ERROR_ON_SENT_DATA 141

// Operational Commands Sending To Play
#define OP_COMMAND_CONFIRM_START_OF_PLAY_DATA 200
#define OP_COMMAND_CONFIRM_START_OF_PLAY_DATA_ROW 201
#define OP_COMMAND_CONFIRM_END_DATA_PLAY_MODE 203
#define OP_COMMAND_ROW_EXECUTION_START 206
#define OP_COMMAND_ROW_EXECUTION_END 207
#define OP_COMMAND_ROW_EXECUTION_PREMATURE_END 208
#define OP_COMMAND_END_OF_PLAY 209
#define OP_COMMAND_CONFIRM_STOP 210
#define OP_COMMAND_ERROR 240
#define OP_COMMAND_ERROR_ON_RECEIVED_DATA 241
#define OP_COMMAND_BATTERY_LEVEL 250
#define OP_COMMAND_FIRMWARE_VERSION 251

// Recieved Message Format
#define MESSAGE_BUFFER_OP_COMMAND 0
#define MESSAGE_BUFFER_ROW 1
#define MESSAGE_BUFFER_THREAD 2
#define MESSAGE_BUFFER_CUBE_TYPE 3
#define MESSAGE_BUFFER_PARAMETERS 4 // 4 - 11 ( 8 Total )
#define MESSAGE_BUFFER_COM_PORT 12
#define MESSAGE_BUFFER_LEVEL 13

enum COMHANDLER_STATE
{
    COMHANDLER_STATE_RUN = 0,
    COMHANDLER_STATE_WAIT,
};

#define TRANSMIT_SERIAL_BUFFER_SIZE             32
/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */
class ComHandler {
    private:
        Crc16 crc;
        byte transmitSerialBuffer[TRANSMIT_SERIAL_BUFFER_SIZE][TOTAL_BYTES_OP_COMMAND_NO_CRC];
        uint8_t tx_widx;
        uint8_t tx_ridx;
        uint32_t tx_timer;
        byte rxDataIDX = 0;  // Current Received Byte
        unsigned long timer = 0; // To mark the last byte Receive time (for timeout)
        boolean isBleConnected(void);
        void sendMsgToPlayCube(byte msg[2]);
		uint32_t battery_timer;
        uint32_t send_timer;
        uint8_t state;

    public:
        byte rxData[SCRIPT_ROW_LENGTH];
        boolean isMessageWaitingForProcess = false;

        ComHandler();
        void processIncomingData();
        void sendStartPlayData();
        void sendEndOfPlayData();
        void sendRowExecute(byte opCommand, byte rowId);
		void sendBatteryLevel(uint16_t level);
		void sendFirmwareVersion(uint8_t fwver);
        void sendRowConfirm(byte row); 
        void sendReceiveDataError(); 
        void sendStopPlayConfirm();
        void sendEndOfPlay();
        void clearSerialBuffer();
};

/* Exported variables ****************************************************** */
extern ComHandler comHandler;

/* Exported functions ****************************************************** */

#endif 
/* ***************************** END OF FILE ******************************* */
