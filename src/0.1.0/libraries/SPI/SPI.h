/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@arduino.cc>
 * Copyright (c) 2014 by Paul Stoffregen <paul@pjrc.com> (Transaction API)
 * Copyright (c) 2014 by Matthijs Kooijman <matthijs@stdin.nl> (SPISettings AVR)
 * Copyright (c) 2014 by Andrew J. Kroll <xxxajk@gmail.com> (atomicity fixes)
 * Copyright (c) 2014 by Andre Moehl andre@ib-moehl.de (SPI1 Class, for Atmega3258PB Support)
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED



#include <Arduino.h>
#include "SPIDef.h"

#define SPI0 SPI //alias, so you can use SPI0.begin() instead of SPI


class SPISettings {
public:
  	SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) 
	{
    	if (__builtin_constant_p(clock)) 
		{
      		init_AlwaysInline(clock, bitOrder, dataMode);
    	} else 
		{
      		init_MightInline(clock, bitOrder, dataMode);
    	}
  	}
  
	SPISettings() 
	{
    	init_AlwaysInline(4000000, MSBFIRST, SPI_MODE0);
  	}

private:
  	void init_MightInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) 
	{
    	init_AlwaysInline(clock, bitOrder, dataMode);
  	}
  	
	void init_AlwaysInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) __attribute__((__always_inline__)) 
	{
		// Clock settings are defined as follows. Note that this shows SPI2X
		// inverted, so the bits form increasing numbers. Also note that
		// fosc/64 appears twice
		// SPR1 SPR0 ~SPI2X Freq
		//   0    0     0   fosc/2
		//   0    0     1   fosc/4
		//   0    1     0   fosc/8
		//   0    1     1   fosc/16
		//   1    0     0   fosc/32
		//   1    0     1   fosc/64
		//   1    1     0   fosc/64
		//   1    1     1   fosc/128

		// We find the fastest clock that is less than or equal to the
		// given clock rate. The clock divider that results in clock_setting
		// is 2 ^^ (clock_div + 1). If nothing is slow enough, we'll use the
		// slowest (128 == 2 ^^ 7, so clock_div = 6).
    	uint8_t clockDiv;

    // When the clock is known at compiletime, use this if-then-else
    // cascade, which the compiler knows how to completely optimize
    // away. When clock is not known, use a loop instead, which generates
    // shorter code.
    if (__builtin_constant_p(clock)) {
      if (clock >= F_CPU / 2) {
        clockDiv = 0;
      } else if (clock >= F_CPU / 4) {
        clockDiv = 1;
      } else if (clock >= F_CPU / 8) {
        clockDiv = 2;
      } else if (clock >= F_CPU / 16) {
        clockDiv = 3;
      } else if (clock >= F_CPU / 32) {
        clockDiv = 4;
      } else if (clock >= F_CPU / 64) {
        clockDiv = 5;
      } else {
        clockDiv = 6;
      }
    } else {
      uint32_t clockSetting = F_CPU / 2;
      clockDiv = 0;
      while (clockDiv < 6 && clock < clockSetting) {
        clockSetting /= 2;
        clockDiv++;
      }
    }

    // Compensate for the duplicate fosc/64
    if (clockDiv == 6)
    clockDiv = 7;

    // Invert the SPI2X bit
    clockDiv ^= 0x1;

    // Pack into the SPISettings class
#if defined(__AVR_ATmega328PB__)
   		spcr = _BV(SPE0) | _BV(MSTR0) | ((bitOrder == LSBFIRST) ? _BV(DORD0) : 0) |
      (dataMode & SPI_MODE_MASK) | ((clockDiv >> 1) & SPI_CLOCK_MASK);
#else
    spcr = _BV(SPE) | _BV(MSTR) | ((bitOrder == LSBFIRST) ? _BV(DORD) : 0) |
      (dataMode & SPI_MODE_MASK) | ((clockDiv >> 1) & SPI_CLOCK_MASK);
    spsr = clockDiv & SPI_2XCLOCK_MASK;
#endif
  }
  uint8_t spcr;
  uint8_t spsr;
  friend class SPIClass;
};



class SPIClass 
{
public:
  // Initialize the SPI library
  static void begin();

  // If SPI is used from within an interrupt, this function registers
  // that interrupt with the SPI library, so beginTransaction() can
  // prevent conflicts.  The input interruptNumber is the number used
  // with attachInterrupt.  If SPI is used from a different interrupt
  // (eg, a timer), interruptNumber should be 255.
  static void usingInterrupt(uint8_t interruptNumber);
  // And this does the opposite.
  static void notUsingInterrupt(uint8_t interruptNumber);
  // Note: the usingInterrupt and notUsingInterrupt functions should
  // not to be called from ISR context or inside a transaction.
  // For details see:
  // https://github.com/arduino/Arduino/pull/2381
  // https://github.com/arduino/Arduino/pull/2449

  // Before using SPI.transfer() or asserting chip select pins,
  // this function is used to gain exclusive access to the SPI bus
  // and configure the correct settings.
  inline static void beginTransaction(SPISettings settings) {
    if (interruptMode > 0) {
      uint8_t sreg = SREG;
      noInterrupts();

      #ifdef SPI_AVR_EIMSK
      if (interruptMode == 1) {
        interruptSave = SPI_AVR_EIMSK;
        SPI_AVR_EIMSK &= ~interruptMask;
        SREG = sreg;
      } else
      #endif
      {
        interruptSave = sreg;
      }
    }

    #ifdef SPI_TRANSACTION_MISMATCH_LED
    if (inTransactionFlag) {
      pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
      digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
    }
    inTransactionFlag = 1;
    #endif

#if defined(__AVR_ATmega328PB__)
    SPCR0 = settings.spcr;
#else
    SPSR = settings.spsr;
#endif
  }

  // Write to the SPI bus (MOSI pin) and also receive (MISO pin)
  inline static uint8_t transfer(uint8_t data) {
#if defined(__AVR_ATmega328PB__)
	SPDR0 = data;
#else
    SPDR = data;
#endif
    /*
     * The following NOP introduces a small delay that can prevent the wait
     * loop form iterating when running at the maximum speed. This gives
     * about 10% more speed, even if it seems counter-intuitive. At lower
     * speeds it is unnoticed.
     */
    asm volatile("nop");
#if defined(__AVR_ATmega328PB__)
    while (!(SPSR0 & _BV(SPIF0))) ; // wait
    return SPDR0;
#else
    while (!(SPSR & _BV(SPIF))) ; // wait
    return SPDR;
#endif
  }
  inline static uint16_t transfer16(uint16_t data) {
    union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } in, out;
    in.val = data;
#if defined(__AVR_ATmega328PB__)
    if (!(SPCR0 & _BV(DORD0))) {
      SPDR0 = in.msb;
      asm volatile("nop"); // See transfer(uint8_t) function
      while (!(SPSR0 & _BV(SPIF0))) ;
      out.msb = SPDR0;
      SPDR0 = in.lsb;
      asm volatile("nop");
      while (!(SPSR0 & _BV(SPIF0))) ;
      out.lsb = SPDR0;
    } else {
      SPDR0 = in.lsb;
      asm volatile("nop");
      while (!(SPSR0 & _BV(SPIF0))) ;
      out.lsb = SPDR0;
      SPDR0 = in.msb;
      asm volatile("nop");
      while (!(SPSR0 & _BV(SPIF0))) ;
      out.msb = SPDR0;
    }
#else
    if (!(SPCR & _BV(DORD))) {
      SPDR = in.msb;
      asm volatile("nop"); // See transfer(uint8_t) function
      while (!(SPSR & _BV(SPIF))) ;
      out.msb = SPDR;
      SPDR = in.lsb;
      asm volatile("nop");
      while (!(SPSR & _BV(SPIF))) ;
      out.lsb = SPDR;
    } else {
      SPDR = in.lsb;
      asm volatile("nop");
      while (!(SPSR & _BV(SPIF))) ;
      out.lsb = SPDR;
      SPDR = in.msb;
      asm volatile("nop");
      while (!(SPSR & _BV(SPIF))) ;
      out.msb = SPDR;
    }
#endif
    return out.val;
  }
  inline static void transfer(void *buf, size_t count) {
    if (count == 0) return;
    uint8_t *p = (uint8_t *)buf;
#if defined(__AVR_ATmega328PB__)
    SPDR0 = *p;
    while (--count > 0) {
      uint8_t out = *(p + 1);
      while (!(SPSR0 & _BV(SPIF0))) ;
      uint8_t in = SPDR0;
      SPDR0 = out;
      *p++ = in;
    }
    while (!(SPSR0 & _BV(SPIF0))) ;
    *p = SPDR0;

#else
    SPDR = *p;
    while (--count > 0) {
      uint8_t out = *(p + 1);
      while (!(SPSR & _BV(SPIF))) ;
      uint8_t in = SPDR;
      SPDR = out;
      *p++ = in;
    }
    while (!(SPSR & _BV(SPIF))) ;
    *p = SPDR;
#endif
  }
  // After performing a group of transfers and releasing the chip select
  // signal, this function allows others to access the SPI bus
  inline static void endTransaction(void) {
    #ifdef SPI_TRANSACTION_MISMATCH_LED
    if (!inTransactionFlag) {
      pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
      digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
    }
    inTransactionFlag = 0;
    #endif

    if (interruptMode > 0) {
      #ifdef SPI_AVR_EIMSK
      uint8_t sreg = SREG;
      #endif
      noInterrupts();
      #ifdef SPI_AVR_EIMSK
      if (interruptMode == 1) {
        SPI_AVR_EIMSK = interruptSave;
        SREG = sreg;
      } else
      #endif
      {
        SREG = interruptSave;
      }
    }
  }

  // Disable the SPI bus
  static void end();

  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  inline static void setBitOrder(uint8_t bitOrder) 
{
#if defined(__AVR_ATmega328PB__)
    if (bitOrder == LSBFIRST) SPCR0 |= _BV(DORD0);
    else SPCR0 &= ~(_BV(DORD0));
#else
    if (bitOrder == LSBFIRST) SPCR |= _BV(DORD);
    else SPCR &= ~(_BV(DORD));
#endif
  }
  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  inline static void setDataMode(uint8_t dataMode) 
 {
#if !defined(__AVR_ATmega328PB__)
    SPCR = (SPCR & ~SPI_MODE_MASK) | dataMode;
#endif
  }
  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  inline static void setClockDivider(uint8_t clockDiv) 
{
#if !defined(__AVR_ATmega328PB__)
    SPCR = (SPCR & ~SPI_CLOCK_MASK) | (clockDiv & SPI_CLOCK_MASK);
    SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((clockDiv >> 2) & SPI_2XCLOCK_MASK);
#endif
  }
  // These undocumented functions should not be used.  SPI.transfer()
  // polls the hardware flag which is automatically cleared as the
  // AVR responds to SPI's interrupt
#if defined(__AVR_ATmega328PB__)
  inline static void attachInterrupt() { SPCR0 |= _BV(SPIE0	); }
  inline static void detachInterrupt() { SPCR0 &= ~_BV(SPIE0); }
#else
  inline static void attachInterrupt() { SPCR |= _BV(SPIE); }
  inline static void detachInterrupt() { SPCR &= ~_BV(SPIE); }
#endif

private:
  static uint8_t initialized;
  static uint8_t interruptMode; // 0=none, 1=mask, 2=global
  static uint8_t interruptMask; // which interrupts to mask
  static uint8_t interruptSave; // temp storage, to restore state
  #ifdef SPI_TRANSACTION_MISMATCH_LED
  static uint8_t inTransactionFlag;
  #endif
};




/* SPI 1 Class *****************************************************/
#if defined(__AVR_ATmega328PB__)
class SPI1Settings {
public:
  	SPI1Settings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) 
	{
    	if (__builtin_constant_p(clock)) 
		{
      		init_AlwaysInline(clock, bitOrder, dataMode);
    	} else 
		{
      		init_MightInline(clock, bitOrder, dataMode);
    	}
  	}
  
	SPI1Settings() 
	{
    	init_AlwaysInline(4000000, MSBFIRST, SPI_MODE0);
  	}

private:
  	void init_MightInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) 
	{
    	init_AlwaysInline(clock, bitOrder, dataMode);
  	}
  	
	void init_AlwaysInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) __attribute__((__always_inline__)) 
	{
		// Clock settings are defined as follows. Note that this shows SPI2X
		// inverted, so the bits form increasing numbers. Also note that
		// fosc/64 appears twice
		// SPR1 SPR0 ~SPI2X Freq
		//   0    0     0   fosc/2
		//   0    0     1   fosc/4
		//   0    1     0   fosc/8
		//   0    1     1   fosc/16
		//   1    0     0   fosc/32
		//   1    0     1   fosc/64
		//   1    1     0   fosc/64
		//   1    1     1   fosc/128

		// We find the fastest clock that is less than or equal to the
		// given clock rate. The clock divider that results in clock_setting
		// is 2 ^^ (clock_div + 1). If nothing is slow enough, we'll use the
		// slowest (128 == 2 ^^ 7, so clock_div = 6).
    	uint8_t clockDiv;

    // When the clock is known at compiletime, use this if-then-else
    // cascade, which the compiler knows how to completely optimize
    // away. When clock is not known, use a loop instead, which generates
    // shorter code.
    if (__builtin_constant_p(clock)) {
      if (clock >= F_CPU / 2) {
        clockDiv = 0;
      } else if (clock >= F_CPU / 4) {
        clockDiv = 1;
      } else if (clock >= F_CPU / 8) {
        clockDiv = 2;
      } else if (clock >= F_CPU / 16) {
        clockDiv = 3;
      } else if (clock >= F_CPU / 32) {
        clockDiv = 4;
      } else if (clock >= F_CPU / 64) {
        clockDiv = 5;
      } else {
        clockDiv = 6;
      }
    } else {
      uint32_t clockSetting = F_CPU / 2;
      clockDiv = 0;
      while (clockDiv < 6 && clock < clockSetting) {
        clockSetting /= 2;
        clockDiv++;
      }
    }

    // Compensate for the duplicate fosc/64
    if (clockDiv == 6)
    clockDiv = 7;

    // Invert the SPI2X bit
    clockDiv ^= 0x1;

    // Pack into the SPI1Settings class
//#if defined(__AVR_ATmega328PB__)
   		spcr = _BV(SPE1) | _BV(MSTR1) | ((bitOrder == LSBFIRST) ? _BV(DORD1) : 0) |
      (dataMode & SPI_MODE_MASK) | ((clockDiv >> 1) & SPI_CLOCK_MASK);
//#endif
  }
  uint8_t spcr;
  uint8_t spsr;
  friend class SPI1Class;
};



class SPI1Class 
{
public:
  // Initialize the SPI library
  static void begin();

  // If SPI is used from within an interrupt, this function registers
  // that interrupt with the SPI library, so beginTransaction() can
  // prevent conflicts.  The input interruptNumber is the number used
  // with attachInterrupt.  If SPI is used from a different interrupt
  // (eg, a timer), interruptNumber should be 255.
  static void usingInterrupt(uint8_t interruptNumber);
  // And this does the opposite.
  static void notUsingInterrupt(uint8_t interruptNumber);
  // Note: the usingInterrupt and notUsingInterrupt functions should
  // not to be called from ISR context or inside a transaction.
  // For details see:
  // https://github.com/arduino/Arduino/pull/2381
  // https://github.com/arduino/Arduino/pull/2449

  // Before using SPI.transfer() or asserting chip select pins,
  // this function is used to gain exclusive access to the SPI bus
  // and configure the correct settings.
  inline static void beginTransaction(SPI1Settings settings) {
    if (interruptMode > 0) {
      uint8_t sreg = SREG;
      noInterrupts();

      #ifdef SPI_AVR_EIMSK
      if (interruptMode == 1) {
        interruptSave = SPI_AVR_EIMSK;
        SPI_AVR_EIMSK &= ~interruptMask;
        SREG = sreg;
      } else
      #endif
      {
        interruptSave = sreg;
      }
    }

    #ifdef SPI_TRANSACTION_MISMATCH_LED
    if (inTransactionFlag) {
      pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
      digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
    }
    inTransactionFlag = 1;
    #endif

//#if defined(__AVR_ATmega328PB__)
    SPCR1 = settings.spcr;
//#endif
  }

  // Write to the SPI bus (MOSI pin) and also receive (MISO pin)
  inline static uint8_t transfer(uint8_t data) {
#if defined(__AVR_ATmega328PB__)
	SPDR1 = data;
#endif
    /*
     * The following NOP introduces a small delay that can prevent the wait
     * loop form iterating when running at the maximum speed. This gives
     * about 10% more speed, even if it seems counter-intuitive. At lower
     * speeds it is unnoticed.
     */
    asm volatile("nop");
//#if defined(__AVR_ATmega328PB__)
    while (!(SPSR1 & _BV(SPIF1))) ; // wait
    return SPDR1;
//#endif
  }

  inline static uint16_t transfer16(uint16_t data) 
  {
    union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } in, out;
    in.val = data;
//#if defined(__AVR_ATmega328PB__)
    if (!(SPCR1 & _BV(DORD1))) {
      SPDR1 = in.msb;
      asm volatile("nop"); // See transfer(uint8_t) function
      while (!(SPSR1 & _BV(SPIF1))) ;
      out.msb = SPDR1;
      SPDR1 = in.lsb;
      asm volatile("nop");
      while (!(SPSR1 & _BV(SPIF1))) ;
      out.lsb = SPDR1;
    } else {
      SPDR1 = in.lsb;
      asm volatile("nop");
      while (!(SPSR1 & _BV(SPIF1))) ;
      out.lsb = SPDR1;
      SPDR1 = in.msb;
      asm volatile("nop");
      while (!(SPSR1 & _BV(SPIF1))) ;
      out.msb = SPDR1;
    }
//#endif
    return out.val;
  }

  inline static void transfer(void *buf, size_t count) 
  {
    if (count == 0) return;
    uint8_t *p = (uint8_t *)buf;
//#if defined(__AVR_ATmega328PB__)
    SPDR1 = *p;
    while (--count > 0) {
      uint8_t out = *(p + 1);
      while (!(SPSR1 & _BV(SPIF1))) ;
      uint8_t in = SPDR1;
      SPDR1 = out;
      *p++ = in;
    }
    while (!(SPSR1 & _BV(SPIF1))) ;
    *p = SPDR1;

//#endif
  }

  // After performing a group of transfers and releasing the chip select
  // signal, this function allows others to access the SPI bus
  inline static void endTransaction(void) 
  {
    #ifdef SPI_TRANSACTION_MISMATCH_LED
    if (!inTransactionFlag) {
      pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
      digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
    }
    inTransactionFlag = 0;
    #endif

    if (interruptMode > 0) {
      #ifdef SPI_AVR_EIMSK
      uint8_t sreg = SREG;
      #endif
      noInterrupts();
      #ifdef SPI_AVR_EIMSK
      if (interruptMode == 1) {
        SPI_AVR_EIMSK = interruptSave;
        SREG = sreg;
      } else
      #endif
      {
        SREG = interruptSave;
      }
    }
  }

  // Disable the SPI bus
  static void end();

  // This function is deprecated.  New applications should use
  // beginTransaction() to configure SPI settings.
  inline static void setBitOrder(uint8_t bitOrder) 
{
//#if defined(__AVR_ATmega328PB__)
    if (bitOrder == LSBFIRST) SPCR1 |= _BV(DORD1);
    else SPCR1 &= ~(_BV(DORD1));
//#endif
  }


  // These undocumented functions should not be used.  SPI.transfer()
  // polls the hardware flag which is automatically cleared as the
  // AVR responds to SPI's interrupt
//#if defined(__AVR_ATmega328PB__)
  inline static void attachInterrupt() { SPCR1 |= _BV(SPIE1	); }
  inline static void detachInterrupt() { SPCR1 &= ~_BV(SPIE1); }
//#endif

private:
  static uint8_t initialized;
  static uint8_t interruptMode; // 0=none, 1=mask, 2=global
  static uint8_t interruptMask; // which interrupts to mask
  static uint8_t interruptSave; // temp storage, to restore state
  #ifdef SPI_TRANSACTION_MISMATCH_LED
  static uint8_t inTransactionFlag;
  #endif
};


#endif




extern SPIClass SPI;
#if defined(__AVR_ATmega328PB__)
extern SPI1Class SPI1;
#endif



#endif
