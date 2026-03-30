/* Includes **************************************************************** */
#include "comHandler.h"
#include "systim.h"
#include "system.h"

/* Private constants ******************************************************* */


/* Private macros ********************************************************** */

/* Private types *********************************************************** */

/* Private variables ******************************************************* */
ComHandler comHandler;

/* Private function prototypes ********************************************* */

/* Exported functions ****************************************************** */
boolean ComHandler::isBleConnected(void) 
{
    return digitalRead(BLE_EN_PIN);
}

void ComHandler::sendMsgToPlayCube(byte msg[2]) 
{
	// if(isBleConnected() != true)
 //    {
 //        debugCOM("BLE is not connected so we are not sending any message\n");
 //        return;
 //    }
    switch(state)
    {
        case(COMHANDLER_STATE_RUN):
        {
            if (!isPlayingFromBrain) 
            {
                // We send feedback on row excution only when the Playing is from the Play Cube
                crc.clearCrc();
                debugCOM(F("Send BLE message["));
                debugCOM(START_OF_MESSAGE_SYMBOL);
                debugCOM(" ");
                debugCOM(START_OF_MESSAGE_SYMBOL);
                debugCOM(" ");
                BLE_UART.write(START_OF_MESSAGE_SYMBOL);
                BLE_UART.write(START_OF_MESSAGE_SYMBOL);

                // Loop over msg to send
                for (int i = 0; i < TOTAL_BYTES_OP_COMMAND_NO_CRC; i++) 
                {
                    BLE_UART.write(msg[i]);
                    debugCOM(msg[i]);
                    debugCOM(" ");
                    crc.updateCrc(msg[i]);
                }

                // Get msg CRC output
                unsigned short crcValue = crc.getCrc();
                byte crcLSB = crcValue;
                byte crcMSB = crcValue >> 8;

                // Send CRC data over serial
                BLE_UART.write(crcLSB);
                BLE_UART.write(crcMSB); 
                BLE_UART.write(END_OF_MESSAGE_SYMBOL_1);
                BLE_UART.write(END_OF_MESSAGE_SYMBOL_2);
                BLE_UART.write(END_OF_MESSAGE_SYMBOL_3);
                debugCOM(crcLSB);
                debugCOM(" ");
                debugCOM(crcMSB,HEX);
                debugCOM(" ");
                debugCOM(END_OF_MESSAGE_SYMBOL_1);
                debugCOM(" ");
                debugCOM(END_OF_MESSAGE_SYMBOL_2);
                debugCOM(" ");
                debugCOM(END_OF_MESSAGE_SYMBOL_3);
                debugCOM(" ");
                debugCOM(F("]\r\n"));
                // state = COMHANDLER_STATE_WAIT;
                // send_timer = getSYSTIM();
            }
            break;
        }
        case(COMHANDLER_STATE_WAIT):
        {
            // This short delay is a must.
            // Without it, the Play doesn't react properly to OP_COMMAND_END_OF_PLAY.
            if (isPlaying) 
            {
                // delay(80);
                if(chk4TimeoutSYSTIM(send_timer,80) == SYSTIM_TIMEOUT)
                {
                    state = COMHANDLER_STATE_RUN;
                }
            }
            else
            {
                state = COMHANDLER_STATE_RUN;
            }
            break;
        }
    }
}


ComHandler::ComHandler() 
{
    pinMode(BLE_EN_PIN, INPUT);
    BLE_UART.begin(BLE_BAUDRATE);
}

void ComHandler::processIncomingData() {
    boolean receivedData = false;

    if(tx_ridx != tx_widx)
    {
        if(chk4TimeoutSYSTIM(tx_timer,80) == SYSTIM_TIMEOUT)
        {
            debugCOM("Send data\n");
            sendMsgToPlayCube(transmitSerialBuffer[tx_ridx]);
            tx_ridx++;
            if(tx_ridx >= TRANSMIT_SERIAL_BUFFER_SIZE)
            {
                tx_ridx = 0;
            }
            tx_timer = getSYSTIM();
        }
    }
    // Check if we timed out - if so we clear the prev buffer in order to get a new one
    // 0 - 255
     
    if ((timer != 0) && (chk4TimeoutSYSTIM(timer,SERIAL_TIME_OUT_MILLIS) == SYSTIM_TIMEOUT) && (rxDataIDX != SCRIPT_ROW_LENGTH)) 
    {
        debugCOM(F("BLE handler timeout. Clearing the serial buffer\r\n"));
        timer = 0;
        clearSerialBuffer();
    }
    // If Ble is connected, listen for ble data
    if (isBleConnected()) 
    {
        if (BLE_UART.available()) 
        {
            rxData[rxDataIDX] = (byte)BLE_UART.read();
            receivedData = true;
        }
    }
    // Else, listen for Micro USB UART Data
    else if (Serial.available()) 
    {
        rxData[rxDataIDX] = (byte)Serial.read();
        receivedData = true;
    }

    if (receivedData) 
    {
        rxDataIDX++;              // Move to next pointer in the buffer.
        timer = getSYSTIM();  // Timeit.
        // sleepTimeoutMillis = getSYSTIM();      // Reset system sleep timeout.
    }

    // If the receiving of data is complete
    if (rxDataIDX == SCRIPT_ROW_LENGTH) 
    {
        // ------------------------------------------------------
        // CHECK CRC
        // ------------------------------------------------------

        crc.clearCrc();
        // Loop over data (with out the last 2 - which are the LSB & MSB of the crc)
        debugCOM("Rec data: ");
        for (int i = 0; i < SCRIPT_ROW_LENGTH; i++) 
        {
            // Add to CRC
            if(i < (SCRIPT_ROW_LENGTH - 2))
            {
                crc.updateCrc(rxData[i]);
            }
            debugCOM(rxData[i],HEX);
        }
        debugCOM("\r\n");

        // Calculate CRC from the received data
        unsigned short crcCalculatedValue = crc.getCrc();
        // Get the pre-calculated CRC from the received msg buffer
        unsigned short crcReceivedValue = (rxData[SCRIPT_ROW_LENGTH - 1] << 8) | rxData[SCRIPT_ROW_LENGTH - 2];

        if (crcReceivedValue == crcCalculatedValue) 
        {
            isMessageWaitingForProcess = true;
        } 
        else 
        {
            debugCOM(F("BLE command has a invalid CRC C["));
            debugCOM(crcCalculatedValue);
            debugCOM(F("]R["));
            debugCOM(crcReceivedValue);
            debugCOM(F("]\r\n"));
            sendReceiveDataError();
        }
    }
}

void ComHandler::sendStartPlayData() {

    transmitSerialBuffer[tx_widx][0] = OP_COMMAND_CONFIRM_START_OF_PLAY_DATA;
    transmitSerialBuffer[tx_widx][1] = rxData[MESSAGE_BUFFER_ROW];
    tx_widx++;
    if(tx_widx >= TRANSMIT_SERIAL_BUFFER_SIZE)
    {
        tx_widx = 0;
    }
    // sendMsgToPlayCube(transmitSerialBuffer);
}

void ComHandler::sendEndOfPlayData() {
    transmitSerialBuffer[tx_widx][0] = OP_COMMAND_CONFIRM_END_DATA_PLAY_MODE;
    transmitSerialBuffer[tx_widx][1] = 0;
    tx_widx++;
    if(tx_widx >= TRANSMIT_SERIAL_BUFFER_SIZE)
    {
        tx_widx = 0;
    }
}

/**
 * @param opCommand is either :
 * - OP_COMMAND_ROW_EXECUTION_START
 * - OP_COMMAND_ROW_EXECUTION_END
 * - OP_COMMAND_ROW_EXECUTION_PREMATURE_END
 * @param rowId is the row that excuted the command
 */
void ComHandler::sendRowExecute(byte opCommand, byte rowId) {
    transmitSerialBuffer[tx_widx][0] = opCommand;
    transmitSerialBuffer[tx_widx][1] = rowId;
    tx_widx++;
    if(tx_widx >= TRANSMIT_SERIAL_BUFFER_SIZE)
    {
        tx_widx = 0;
    }
    tx_timer = getSYSTIM();
}

void ComHandler::sendBatteryLevel(uint16_t level) 
{
	if(isBleConnected() == true)
    {
        transmitSerialBuffer[tx_widx][0] = OP_COMMAND_BATTERY_LEVEL;
        uint8_t decimal = level / 100;
        uint8_t fractional = (level - (decimal * 100)) / 10;
        uint8_t value = ((decimal & 0x0f) << 4) | (fractional & 0xf);
        transmitSerialBuffer[tx_widx][1] = value;
        tx_widx++;
        if(tx_widx >= TRANSMIT_SERIAL_BUFFER_SIZE)
        {
            tx_widx = 0;
        }

    }
}
void ComHandler::sendFirmwareVersion(uint8_t fwver) 
{
    if(isBleConnected() == true)
    {
        transmitSerialBuffer[tx_widx][0] = OP_COMMAND_FIRMWARE_VERSION;
        transmitSerialBuffer[tx_widx][1] = fwver;
        tx_widx++;
        if(tx_widx >= TRANSMIT_SERIAL_BUFFER_SIZE)
        {
            tx_widx = 0;
        }

    }
}


void ComHandler::sendRowConfirm(byte row) {
    transmitSerialBuffer[tx_widx][0] = OP_COMMAND_CONFIRM_START_OF_PLAY_DATA_ROW;
    transmitSerialBuffer[tx_widx][1] = row;
    tx_widx++;
    if(tx_widx >= TRANSMIT_SERIAL_BUFFER_SIZE)
    {
        tx_widx = 0;
    }
}

void ComHandler::sendReceiveDataError() {
    transmitSerialBuffer[tx_widx][0] = OP_COMMAND_ERROR_ON_RECEIVED_DATA;
    transmitSerialBuffer[tx_widx][1] = 0;
    tx_widx++;
    if(tx_widx >= TRANSMIT_SERIAL_BUFFER_SIZE)
    {
        tx_widx = 0;
    }
}

void ComHandler::sendStopPlayConfirm() {
    transmitSerialBuffer[tx_widx][0] = OP_COMMAND_CONFIRM_STOP;
    transmitSerialBuffer[tx_widx][1] = 0;
    tx_widx++;
    if(tx_widx >= TRANSMIT_SERIAL_BUFFER_SIZE)
    {
        tx_widx = 0;
    }
    // delay(2); // short delay for mesaage sending
    // sendMsgToPlayCube(transmitSerialBuffer[tx_widx]);
    // delay(2);
}

void ComHandler::sendEndOfPlay() {
    transmitSerialBuffer[tx_widx][0] = OP_COMMAND_END_OF_PLAY;
    transmitSerialBuffer[tx_widx][1] = 0;
    tx_widx++;
    if(tx_widx >= TRANSMIT_SERIAL_BUFFER_SIZE)
    {
        tx_widx = 0;
    }
}

void ComHandler::clearSerialBuffer() 
{
    for (int i = 0; i < SCRIPT_ROW_LENGTH; i++) 
    {
        rxData[i] = 0;
    }
    rxDataIDX = 0;
    timer = 0;
}
/* Private functions ******************************************************* */

/* ***************************** END OF FILE ******************************* */
