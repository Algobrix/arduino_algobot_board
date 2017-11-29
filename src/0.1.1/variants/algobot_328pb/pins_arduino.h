/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.h 249 2007-02-03 16:52:51Z mellis $
*/


/* *algobot - Atmega328PB based Board with 16MHz 
* by ofer zvik 2017
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

#define NUM_DIGITAL_PINS            24
#define NUM_ANALOG_INPUTS           8


/*pins which can deliver PWM */
#define digitalPinHasPWM(p)   (((p) >= 0 && (p) <= 3) || ((p) >= 10 && (p)<= 13) || (p) == 6)

/* First SPI */
static const uint8_t SS   = 12;
static const uint8_t MOSI = 13;
static const uint8_t MISO = 14;
static const uint8_t SCK  = 15;

/* 2nd SPI */
static const uint8_t SS1   = 16;
static const uint8_t MOSI1 = 17;
static const uint8_t MISO1 = 18;
static const uint8_t SCK1  = 19;


/* Default I2C is I2C1, because I2C0 is at the additionally pin header */
static const uint8_t SDA = 20;	// PC4
static const uint8_t SCL = 21;	// PC5

// IGONRE FOR NOW ....
//static const uint8_t SDA0 = 21;	//PC4
//static const uint8_t SCL0 = 20; //PC5

/* Second I2C */
//static const uint8_t SDA1 = 8;	// PE0
//static const uint8_t SCL1 = 7;	// PE1

#undef LED_BUILTIN
#define LED_BUILTIN 	15




/*Pin Change Interrupt Control Register */
#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= NUM_DIGITAL_PINS) ? (&PCICR) : ((uint8_t *)0))
#define digitalPinToPCICRbit(p) \
	( \
		(p) == 0 ? 2 : \
		((p) == 1 ? 2 : \
	 	((p) == 2 ? 2 : \
		((p) == 3 ? 2 : \
		((p) == 4 ? 2 : \
		((p) == 5 ? 0 : \
		((p) == 6 ? 0 : \
		((p) == 7 ? 3 : \
		((p) == 8 ? 3 : \
		((p) == 9 ? 2 : \
		((p) == 10 ? 2 : \
		((p) == 11 ? 2 : \
		((p) == 12 ? 0 : \
		((p) == 13 ? 0 : \
		((p) == 14 ? 0 : \
		((p) == 15 ? 0 : \
		((p) == 16 ? 3 : \
		((p) == 17 ? 3 : \
		((p) == 18 ? 1 : \
		((p) == 19 ? 1 : \
		((p) == 20 ? 1 : \
		((p) == 21 ? 1 : \
		((p) == 22 ? 1 : \
		((p) == 23 ? 1 : 0) \
	)))))))))))))))))))))))

/* Pin Change Mask Register 0*/
// Pin Change interrupts
// PCINT0 - PCINT5 = PB0-PB5	PCMSK0
// PCINT8 - PCINT13 = PC0-PC5	PCMSK1
// PCINT16 - PCINT23 = PD0-PD7	PCMSK2
// PCINT24 - PCINT27 = PE0-PE3	PCMSK3

/* map digital pin to pin change mask register */
#define digitalPinToPCMSK(p)  (((p) == 5 || (p) == 6 || (p) == 13 || (p) == 14 || (p) == 15 || (p) == 16 ) ? &PCMSK0: \
								(((p) >= 18 && p <= 23) ? &PCMSK1 : \
								 ((((p) >= 0 && (p) <= 4) || ((p) >= 9 && (p) <= 11))  ? &PCMSK2: \
								  (((p) == 7 || (p) == 8 || (p) == 16 || (p) == 17 ) ? &PCMSK3: ((uint8_t *)0) \
								))))

/* map arduino pin to bit of the PCMSK Register */
#define digitalPinToPCMSKbit(p) \
	( \
		(p) == 0 ? 1 : \
		((p) == 1 ? 0 : \
	 	((p) == 2 ? 5 : \
		((p) == 3 ? 6 : \
		((p) == 4 ? 7 : \
		((p) == 5 ? 0 : \
		((p) == 6 ? 1 : \
		((p) == 7 ? 1 : \
		((p) == 8 ? 0 : \
		((p) == 9 ? 4 : \
		((p) == 10 ? 3 : \
		((p) == 11 ? 2 : \
		((p) == 12 ? 2 : \
		((p) == 13 ? 3 : \
		((p) == 14 ? 4 : \
		((p) == 15 ? 5 : \
		((p) == 16 ? 2 : \
		((p) == 17 ? 3 : \
		((p) == 18 ? 0 : \
		((p) == 19 ? 1 : \
		((p) == 20 ? 4 : \
		((p) == 21 ? 5 : \
		((p) == 22 ? 3 : \
		((p) == 23 ? 2 : 0) \
	)))))))))))))))))))))))


/* map arduino pins to external interrupt */
#define digitalPinToInterrupt(p)  ((p) == 11 ? 0 : ((p) == 10 ? 1 : NOT_AN_INTERRUPT))


/* Analog Pins*/
#define analogInputToDigitalPin(p)  ((p < NUM_ANALOG_INPUTS) ? (p) + 16 : -1)
static const uint8_t A0 = 0;
static const uint8_t A1 = 1;
static const uint8_t A2 = 2;
static const uint8_t A3 = 3;
static const uint8_t A4 = 4;
static const uint8_t A5 = 5;
static const uint8_t A6 = 6;
static const uint8_t A7 = 7;

/* Analog Channel Pin Assignment */
extern const uint8_t PROGMEM analog_pin_to_channel_PGM[];
#define analogPinToChannel(P)  ( pgm_read_byte( analog_pin_to_channel_PGM + (P) ) )

#ifdef ARDUINO_MAIN


// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
	(uint16_t) &DDRE,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
	(uint16_t) &PORTE,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
	(uint16_t) &PINE,
};

/* Port Settings per externel IO Pin*/
const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
	PD, // PD1 /* Left Side Top*/
	PD, // PD0 
	PD, // PD5
	PD, // PD6
	PD, // PD7
	PB, // PB0
	PB, // PB1
	PE, // PE1
	PE, // PE0
	PD, // PD4
	PD, // PD3
	PD, // PD2 /* Left Side Bottom*/
	PB, // PB2 /* Right Side Bottom */
	PB, // PB3
	PB, // PB4
	PB, // PB5 // LED
	PE, // PE2
	PE, // PE3
	PC, // PC0
	PC, // PC1  /* Right Side Top */
	PC,	// PC4
	PC, // PC5	/* External Connector Left*/
	PC, // PC3
	PC  // PC2 /* External Connector Right*/
};

/* Pin - Port Setting*/
const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	_BV(1), 
	_BV(0),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(0),
	_BV(1),
	_BV(1),
	_BV(0), 
	_BV(4),
	_BV(3),
	_BV(2),
	_BV(2),
	_BV(3),
	_BV(4), 
	_BV(5),
	_BV(2),
	_BV(3),
	_BV(0),
	_BV(1),
	_BV(4),
	_BV(5),
	_BV(3),
	_BV(2)
};

/* Pin-Timer Assignment */
const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	TIMER4A, 
	TIMER3A,
	TIMER0B,
	TIMER0A,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	TIMER1A,
	NOT_ON_TIMER,
	NOT_ON_TIMER, 
	NOT_ON_TIMER,
	TIMER2B,
	TIMER4B,
	TIMER1B,
	TIMER2A,
	NOT_ON_TIMER,
	NOT_ON_TIMER, 
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER
};

/*
const uint8_t PROGMEM analog_pin_to_channel_PGM[] = {
    6,  // A0               PE2                 ADC6
    1,  // A1               PF3                 ADC1    
    0,  // A2               PC0                 ADC0    
    1,  // A3               PC1                 ADC7
    5,  // A4               PC5                 ADC5    
    4,  // A5               PC4                 ADC4    
    3,  // A6               PC3                 ADC3
    2,  // A7               PC2                 ADC2
};
*/
const uint8_t PROGMEM analog_pin_to_channel_PGM[] = {
	0,  // A0               PC0                 ADC0
    1,  // A1               PC1                 ADC1    
    2,  // A2               PC2                 ADC2    
    3,  // A3               PC3                 ADC3
    4,  // A4               PC4                 ADC4    
    5,  // A5               PC5                 ADC5    
    6,  // A6               PE2                 ADC6
    7,  // A7               PE3                 ADC7
};

#endif /* ARDUINO_MAIN */


#define SERIAL_PORT_MONITOR   		Serial
#define SERIAL_PORT_HARDWARE  		Serial
#define SERIAL_PORT_HARDWARE1       Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial1

#endif
