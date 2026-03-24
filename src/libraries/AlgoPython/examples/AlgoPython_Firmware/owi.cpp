#include "owi.h"

#define OWI_RESET_LOW                   480
#define OWI_PRESENCE_SAMPLE             70
#define OWI_WAIT_AFTER_RESET            410

#define OWI_BIT_PERIOD                  80
#define OWI_BIT_ONE_LOW                 1
#define OWI_BIT_ONE_HIGH                64 
#define OWI_BIT_ZERO_LOW                62
#define OWI_BIT_ZERO_HIGH               10 

#define OWI_BIT_READ_LOW                6
#define OWI_BIT_SAMPLE_TIME             9
#define OWI_BIT_WAIT_AFTER_READ         55 

OWI::OWI(uint8_t pin, uint8_t * pinr, uint8_t * port, uint8_t * ddr)
{
    pin_ = pin;
    pinr_ = pinr;
    port_ = port;
    ddr_ = ddr;

    this->pinInput();
}

void OWI::pinOutput(void)
{
    *ddr_ |= (1 << pin_);
}
void OWI::pinInput(void)
{
    *ddr_ &= ~(1 << pin_);
}
void OWI::pinHigh(void)
{
    *port_ |= (1 << pin_);

}
void OWI::pinLow(void)
{
    *port_ &= ~(1 << pin_);
}

uint8_t OWI::pinRead(void)
{
    return (*pinr_ & (1 << pin_)) ? 1 : 0;
}


void OWI::reset(void)
{
    this->pinOutput();
    this->pinLow();
    delayMicroseconds(OWI_RESET_LOW);
    this->pinHigh();
    this->pinInput();
}

uint8_t OWI::wait4Presence(void)
{
    volatile uint8_t status = 0;
    this->pinInput();
    delayMicroseconds(OWI_PRESENCE_SAMPLE);
    volatile uint8_t level = this->pinRead();
    if(level == 0)  
    {
        status = 0;
    }
    else
    {
        status = 1;
    }
    delayMicroseconds(OWI_WAIT_AFTER_RESET);
    return status;
}

void OWI::txByte(uint8_t data)
{
    uint8_t k = 0;
    PORTB ^= (1 << 5);
    for( k = 0; k < 8; k++ )
    {
        this->pinOutput();
        this->pinLow();
        delayMicroseconds(OWI_BIT_ONE_LOW);
        if(data & 0x01)
        {
            this->pinInput();
        }
        delayMicroseconds(OWI_BIT_ZERO_LOW);
        this->pinInput();
        delayMicroseconds(2);
        data = data >> 1;
    }
    delayMicroseconds(10);
}
uint8_t OWI::rxByte(void)
{
    uint8_t data = 0x00;
    uint8_t k = 0;
    for( k = 0; k < 8; k++ )
    {
        data = data >> 1;
        this->pinOutput();
        this->pinLow();
        delayMicroseconds(OWI_BIT_READ_LOW);
        this->pinInput();
        delayMicroseconds(OWI_BIT_SAMPLE_TIME);
        if(this->pinRead())
        {
            data |= 0x80;
        }
        delayMicroseconds(OWI_BIT_WAIT_AFTER_READ);
    }
    return data;
}


#define OWI_ROM_SIZE 8
#define OWI_READ_ROM 0x33
#define OWI_MATCH_ROM 0x55
#define OWI_SKIP_ROM 0xcc
#define OWI_SEARCH_ROM 0xf0
#define OWI_ALARM_SEARCH 0xec

#define OWI_WRITE 0x4e
#define OWI_READ 0xbe
#define OWI_COPY 0x48
#define OWI_CONVERT 0x44
#define OWI_RECALL_E2 0xb8
#define OWI_RPS 0x34

uint8_t OWI::readValue(uint8_t reg, uint8_t *value)
{
  this->reset();
  if(this->wait4Presence() != 0)
  {
      Serial.println("Presence not detected\r\n");
      return 0;
  }
  delay(10);
  this->txByte(OWI_SKIP_ROM);
  this->txByte(reg);
  *value = this->rxByte();
  return 1;
}
