//-------------------------------------------------------------------------------------
// CRC16 support class
// Based on various examples found on the web
// Copyright (C) 2014 Vincenzo Mennella (see license.txt)
// History
//  0.1.0 31/05/2014:   First public code release
//  0.1.1 17/12/2014:   Minor revision and commented code
//
// License
// "MIT Open Source Software License":
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in the
// Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//-------------------------------------------------------------------------------------
#ifndef CRC16_H
#define CRC16_H
#define LIBRARY_VERSION_CRC16_H   "0.1.1"

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

class Crc16 {
   private:
        //Crc parameters
        uint16_t _msbMask;
        uint16_t _mask;
        uint16_t _xorIn;
        uint16_t _xorOut;
        uint16_t _polynomial;
        uint8_t _reflectIn;
        uint8_t _reflectOut;
        //Crc value
		uint16_t _crc;
		uint8_t reflect(uint8_t data, uint8_t bits = 32);

   public:
        inline Crc16()
        {
            //Default to XModem parameters
            _reflectIn = false;
            _reflectOut = false;
            _polynomial = 0x1021;
            _xorIn = 0x0000;
            _xorOut = 0x0000;
            _msbMask = 0x8000;
            _mask = 0xFFFF;
        }
        inline Crc16(uint8_t reflectIn, uint8_t reflectOut, uint16_t polynomial, uint16_t xorIn, uint16_t xorOut, uint16_t msbMask, uint16_t mask)
        {
            _reflectIn = reflectIn;
            _reflectOut = reflectOut;
            _polynomial = polynomial;
            _xorIn = xorIn;
            _xorOut = xorOut;
            _msbMask = msbMask;
            _mask = mask;
        }
		void clearCrc();
		void updateCrc(uint8_t data);
		uint16_t getCrc();
		unsigned int fastCrc(uint8_t data[], uint8_t start, uint16_t length, uint8_t reflectIn, uint8_t reflectOut, uint16_t polynomial, uint16_t xorIn, uint16_t xorOut, uint16_t msbMask, uint16_t mask);
		inline unsigned int XModemCrc(uint8_t data[], uint8_t start, uint16_t length)
		{
            //  XModem parameters: poly=0x1021 init=0x0000 refin=false refout=false xorout=0x0000
            return fastCrc(data, start, length, false, false, 0x1021, 0x0000, 0x0000, 0x8000, 0xffff);
		}
};

#endif
