/* Includes **************************************************************** */
#include "comHandler.h"
#include "systim.h"
#include "system.h"
#include "fwver.h"
#include "runTime.h"
#include "sensor.h"
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
/* AlgoPython section ****************************************************** */
void ComHandler::sendReply(uint8_t cmd) {
    Serial.write(0xA5);        // Start byte
    Serial.write(cmd);         // Command byte
    Serial.write(0x00);        // Size = 0 (no payload)

    uint8_t crc = (0xA5 + cmd + 0x00) % 256;
    Serial.write(crc);         // CRC
}

void ComHandler::sendReply(uint8_t cmd, const uint8_t* payload, uint8_t size) {
    Serial.write(0xA5);        // Start byte
    Serial.write(cmd);         // Command byte
    Serial.write(size);        // Payload size

    uint8_t crc = 0xA5 + cmd + size;

    for (uint8_t i = 0; i < size; i++) {
        Serial.write(payload[i]);
        crc += payload[i];
    }

    Serial.write(crc % 256);   // CRC
}


void ComHandler::processPythonCommands() {
    while (Serial.available()) {
        uint8_t byte = Serial.read();

        if (rxIndex == 0 && byte != 0xA5) {
            // Wait for start byte
            continue;
        }

        rxBuffer[rxIndex++] = byte;

        if (rxIndex == 3) {
            // We just received the SIZE byte
            uint8_t size = rxBuffer[2];
            if (size > MAX_PAYLOAD) {
                Serial.print("Payload too big: ");
                Serial.println(size);
                rxIndex = 0;  // invalid size
                return;
            }
        }

        if (rxIndex >= 3) {
            uint8_t expectedLength = 3 + rxBuffer[2] + 1;
            if (rxIndex == expectedLength) {
 
                uint8_t sum = 0;
                for (uint8_t i = 0; i < expectedLength - 1; i++) {
                    sum += rxBuffer[i];
                }
                uint8_t calculatedCrc = (rxBuffer[0] + rxBuffer[1] + rxBuffer[2]) % 256;
                uint8_t expectedCrc = rxBuffer[expectedLength - 1];

                if (calculatedCrc == expectedCrc) {
                    packetReady = true;
                } else {
                    Serial.println("Invalid CRC from Python");
                    Serial.print("Expected CRC: 0x");
                    Serial.println(expectedCrc, HEX);
                    Serial.print("Calculated CRC: 0x");
                    Serial.println(calculatedCrc, HEX);
                }

                rxIndex = 0;
                return;
            }
        }
    }

    if (packetReady) {
        parseAndHandleCommand();
        packetReady = false;
    }
}


void ComHandler::parseAndHandleCommand() 
{
    uint8_t cmd = rxBuffer[1];
    uint8_t size = rxBuffer[2];
    uint8_t* payload = &rxBuffer[3];

    switch (cmd) {
        case ALGOPYTHON_CMD_MOVE_REQ:
        {
            if (size >= 8) 
            {
                uint8_t motor_port = payload[0];
                uint8_t motor_type = payload[1];
                uint32_t motor_duration = payload[2];
                motor_duration = (motor_duration << 8) | payload[3];
                motor_duration = (motor_duration << 8) | payload[4];
                motor_duration = (motor_duration << 8) | payload[5];
                uint8_t motor_power = payload[6];
                uint8_t motor_dir = payload[7];
                cmd_process_move(motor_port, motor_type, motor_duration/100. , motor_power, motor_dir);
                sendReply(ALGOPYTHON_CMD_MOVE_REP);
            }
            break;
        }  
        case ALGOPYTHON_CMD_LIGHT_REQ:
        {
            if (size >= 10) {
                uint8_t led_port = payload[0];
                uint8_t led_type = payload[1];
                uint32_t led_duration = payload[2];
                led_duration = (led_duration << 8) | payload[3];
                led_duration = (led_duration << 8) | payload[4];
                led_duration = (led_duration << 8) | payload[5];
                uint8_t led_power = payload[6];
                uint8_t led_r = payload[7];
                uint8_t led_g = payload[8];
                uint8_t led_b = payload[9];

                cmd_process_light(led_port,led_type,led_duration/100.,led_power,  led_r, led_g, led_b);
                sendReply(ALGOPYTHON_CMD_LIGHT_REP);
            }
            break;
        }
        case ALGOPYTHON_CMD_PLAY_SOUND_REQ:
        {
            if (size >= 2) {
                uint8_t sound_id = payload[0];
                uint8_t volume = payload[1];
                cmd_process_playSound(sound_id, volume);
                sendReply(ALGOPYTHON_CMD_PLAY_SOUND_REP);
            }
            break;
        }
        case ALGOPYTHON_CMD_MOVE_STOP_REQ:
        {
            uint8_t motor_port = payload[0];
            cmd_process_move_stop(motor_port);
            sendReply(ALGOPYTHON_CMD_MOVE_STOP_REP);
            break;
        }
        case ALGOPYTHON_CMD_LIGHT_STOP_REQ:
        {
            uint8_t light_port = payload[0];
            cmd_process_light_stop(light_port);
            sendReply(ALGOPYTHON_CMD_LIGHT_STOP_REP);
            break;
        }
        case ALGOPYTHON_CMD_SOUND_STOP_REQ:
        {
            cmd_process_sound_stop();
            sendReply(ALGOPYTHON_CMD_SOUND_STOP_REP);
            break;
        }
        case ALGOPYTHON_CMD_WAIT_SENSOR_REQ:
        {
            if (size >= 3) {
                uint8_t sensor_port = payload[0];
                uint8_t min = payload[1];
                uint8_t max = payload[2];

                cmd_process_wait_sensor(sensor_port, min, max);
                sendReply(ALGOPYTHON_CMD_WAIT_SENSOR_REP);
            }
            break;
        }  
        case ALGOPYTHON_CMD_GET_STATUS_REQ: 
        {
            uint8_t statusPayload[10];
            fillStatusPayload(statusPayload);
            sendReply(ALGOPYTHON_CMD_GET_STATUS_REP, statusPayload, sizeof(statusPayload));
            break;
        }
        default:
            Serial.println("Unknown CMD from Python");
            break;
    }
}

/* AlgoPython section ****************************************************** */
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
