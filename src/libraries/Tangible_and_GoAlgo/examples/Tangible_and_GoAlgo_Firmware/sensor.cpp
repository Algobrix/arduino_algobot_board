/* Includes **************************************************************** */
#include "sensor.h"
#include <SoftwareSerial.h>
#include "systim.h"
#include "config.h"


/* Private constants ******************************************************* */


/* Private macros ********************************************************** */

/* Private types *********************************************************** */

/* Private variables ******************************************************* */
Sensor sensors[NUM_OF_SENSORS] = {Sensor(SENSOR_A_PIN), Sensor(SENSOR_B_PIN)};


/* Private function prototypes ********************************************* */

/* Exported functions ****************************************************** */
Sensor::Sensor(byte pin): serial(pin) {
    this->pin = pin;
    pinMode(pin, INPUT);
    //serialtx.begin(4800);
}


uint8_t Sensor::getValue(uint8_t *sensor_type) {
    float dutyCycle = 0;
    uint8_t value = 0;
    *sensor_type = this->type;
    if(this->type == ALGOSENSOR_TYPE_1WIRE)
    {
#ifdef USE_SENSOR_RAW_VALUE
        cmd_tx(0x20,NULL,0);
#else
        cmd_tx(0x30,NULL,0);
#endif
        uint8_t cmd;
        uint8_t payload[32];
        uint8_t size;
        int8_t res = cmd_get_response(&cmd,payload,&size);
        Serial.println("res");
        if(res == 0)
        {
            uint32_t raw_value = payload[0];
            raw_value = (raw_value << 8) | payload[1];
            raw_value = (raw_value << 8) | payload[2];
            raw_value = (raw_value << 8) | payload[3];
            value = raw_value;
        }
        else
        {
			pinMode(this->pin,INPUT_PULLUP);
            uint32_t cnt = 10000;
            while(cnt--)
            {
                if (digitalRead(pin) == 0)
                {
                    this->type = ALGOSENSOR_TYPE_PWM;
                    *sensor_type = ALGOSENSOR_TYPE_PWM;
                    debugSENSOR(F("PWM device detected\r\n"));
                    break;
                }
                delayMicroseconds(10);
            }
            return 0;
        }
    }
    else
    {
      //debugSENSOR("PWM\r\n");
        unsigned long pwmHighValue = pulseIn(pin, HIGH, PULSE_TIMEOUT);
        uint8_t dutyCycle0;
        if(pwmHighValue != 0)
        {
            dutyCycle = (float(pwmHighValue) / CYCLE_TIME) * 100.0f;
            dutyCycle0 = dutyCycle;
        }
        else if (digitalRead(pin))
        {
            this->type = ALGOSENSOR_TYPE_1WIRE;
            *sensor_type = ALGOSENSOR_TYPE_1WIRE;
            dutyCycle = 0;
            debugSENSOR(F("1Wire device detected\r\n"));;
        } 
        else
        {
            dutyCycle = 0;
            value = uint8_t(round(dutyCycle / 10));
        }
        if(pwmHighValue)
        {
            uint8_t k = 0;
            for(k = 0; k < 20; k++)
            {
                uint32_t value = pulseIn(pin, HIGH, 4000);
                if(value == 0)
                {
                    break;
                }
            }
            if(k < 20)
            {
                Serial.print("Sound sensor new: ");
                Serial.println(k);
                value = 1;
            }
            else
            {
                value = uint8_t(round(dutyCycle / 10));
            }
        }
        else
        {
            value = uint8_t(round(dutyCycle / 10));
        }
    }
    static int old_value = 0;
    if(old_value != value)
    {
        debugSENSOR(F("Sensor value:  "));
        debugSENSOR(value);
        debugSENSOR(F("\r\n"));
        old_value = value;
    }

    return value;
}


void Sensor::cmd_tx(uint8_t cmd, uint8_t *payload, uint8_t size) {
    uint8_t data[32];
    uint8_t crc = 0;
    uint8_t k = 0;

    // Construct message
    data[k++] = 0xA5;  // Preamble
    crc += 0xA5;
    data[k++] = cmd;
    crc += cmd;
    data[k++] = size;
    crc += size;

    for (uint8_t p = 0; p < size; p++) {
        data[k++] = payload[p];
        crc += payload[p];
    }

    crc = ~crc + 1;   // Calculate CRC
    data[k++] = crc;

    // Transmit data array using SoftwareUART write method
    serial.write(data, k);
}

int8_t Sensor::cmd_get_response(uint8_t *cmd, uint8_t *payload, uint8_t *size) 
{
    uint8_t buffer[32];          // Temporary buffer to store the response
    uint8_t crc = 0x00;
    uint8_t preamble, cmd_id, cmd_size, cmd_crc;
    uint8_t bytesReceived = 0;
    unsigned long initialTimeout = 100000;  // 100 ms for the first byte (in microseconds)
    unsigned long byteTimeout = 5000;       // 5 ms for subsequent bytes (in microseconds)
    unsigned long startTime = micros();

    // Wait for the first byte with a 100 ms timeout
    if (serial.rx_byte(&preamble, initialTimeout) != 0) {
        Serial.println("Error: Timeout on first byte.");
        return -1;  // Timeout on first byte
    }

    // Start accumulating bytes
    buffer[bytesReceived++] = preamble;

    // Check for the correct preamble
    if (preamble != 0xA5) {
        Serial.println("Error: Invalid preamble.");
        return -3;  // Invalid preamble
    }
    crc += preamble;

    // Receive the rest of the message, using a 5 ms timeout for each byte
    while (bytesReceived < sizeof(buffer)) {
        if (serial.rx_byte(&buffer[bytesReceived], byteTimeout) != 0) {
            break;  // Timeout on subsequent byte (indicates end of packet)
        }
        bytesReceived++;
    }

    // Verify that we received at least enough bytes for preamble, cmd, size, and crc
    if (bytesReceived < 4) {
        Serial.println("Error: Not enough data received.");
        return -1;  // Not enough data received
    }

    // Parse the received data
    cmd_id = buffer[1];
    cmd_size = buffer[2];
    *cmd = cmd_id;
    *size = cmd_size;
    crc += cmd_id + cmd_size;

    // Check if the entire payload is present
    if (bytesReceived < (3 + cmd_size + 1)) {
        Serial.println("Error: Incomplete payload.");
        return -1;  // Not enough data for payload and CRC
    }

    // Extract the payload and calculate CRC
    for (uint8_t i = 0; i < cmd_size; i++) {
        payload[i] = buffer[3 + i];
        crc += payload[i];
    }

    // Validate CRC
    cmd_crc = buffer[3 + cmd_size];
    crc = (~crc) + 1;
    if (crc != cmd_crc) {
        Serial.println("Error: CRC mismatch.");
        return -2;  // CRC error
    }

#ifdef DEBUG
    // Print received data if successful
    Serial.print("Received Packet: ");
    Serial.print("Preamble: 0x"); Serial.print(preamble, HEX);
    Serial.print(" Command: 0x"); Serial.print(cmd_id, HEX);
    Serial.print(" Size: "); Serial.print(cmd_size);
    Serial.print(" Payload: ");
    for (uint8_t i = 0; i < cmd_size; i++) {
        Serial.print("0x"); Serial.print(payload[i], HEX); Serial.print(" ");
    }
    Serial.print(" CRC: 0x"); Serial.println(cmd_crc, HEX);
#endif
    return 0;  // Success
}


SoftwareUART::SoftwareUART(uint8_t rxPin) : rxPin(rxPin) {
    pinMode(rxPin, INPUT_PULLUP);
}

int SoftwareUART::rx_byte(uint8_t* value, unsigned long timeout) 
{
    uint8_t receivedByte = 0;
    unsigned long startTime = micros();

    // Wait for the start bit within the timeout period
    while (digitalRead(rxPin) == HIGH) { // Wait for LOW (start bit)
        if ((micros() - startTime) >= timeout) {
            return -1; // Timeout error
        }
    }

    // Wait for the middle of the start bit
    delayMicroseconds(bitPeriod / 2);

    // Read each bit in the byte
    for (int i = 0; i < 8; i++) {
        delayMicroseconds(bitPeriod); // Wait for one full bit period
        if (digitalRead(rxPin) == HIGH) { // Check the bit value
            receivedByte |= (1 << i); // Set the corresponding bit
        }
    }

    // Wait for the stop bit (logic HIGH)
    delayMicroseconds(bitPeriod);

    *value = receivedByte; // Store the received byte in the provided variable
    return 0; // Successful reception
}

void SoftwareUART::tx_byte(uint8_t value) 
{
   // Array to store the entire transmission sequence (1 start bit, 8 data bits, 1 stop bit)
    uint8_t transmissionBits[10];

    // Prepare the transmission sequence
    transmissionBits[0] = LOW; // Start bit
    for (uint8_t bit = 0; bit < 8; bit++) {
        transmissionBits[bit + 1] = (value & (1 << bit)) ? HIGH : LOW; // Data bits (LSB first)
    }
    transmissionBits[9] = HIGH; // Stop bit

    // Configure TX pin for output
    digitalWrite(rxPin, HIGH); // Ensure idle state
    pinMode(rxPin, OUTPUT);

    cli(); 
    // Transmit the precomputed sequence
    for (uint8_t i = 0; i < 10; i++) {
        digitalWrite(rxPin, transmissionBits[i]);
        delayMicroseconds(bitPeriod);
    }
    sei();

    // Switch back to RX mode (if needed for one-wire communication)
    pinMode(rxPin, INPUT_PULLUP);
}


void SoftwareUART::write(uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        tx_byte(data[i]);  // Transmit each byte in the array
    }
}

/* Private functions ******************************************************* */

/* ***************************** END OF FILE ******************************* */
