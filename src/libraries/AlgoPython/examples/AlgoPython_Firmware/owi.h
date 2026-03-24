/**
 * @file        owi.h
 * @author      Semir Tursunovic (semir-t) 
 * @date        December 2022
 * @version     1.0.0
 */

/* Define to prevent recursive inclusion *********************************** */
#ifndef __OWI_H
#define __OWI_H

/* Includes **************************************************************** */
#include <Arduino.h>

/* Exported constants ****************************************************** */

/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */
class OWI
{
    private:
        uint8_t pin_;
        uint8_t * pinr_;
        uint8_t * port_;
        uint8_t * ddr_;
    public:
        OWI(uint8_t pin, uint8_t * pinr, uint8_t * port, uint8_t * ddr);
        void pinHigh(void);
        void pinLow(void);
        void pinOutput(void);
        void pinInput(void);
        uint8_t pinRead(void);
        void reset(void);
        uint8_t wait4Presence(void);
        void txByte(uint8_t data);
        uint8_t rxByte(void);
        uint8_t readValue(uint8_t reg,uint8_t * value);
};

/* Exported variables ****************************************************** */

/* Exported functions ****************************************************** */

#endif 
/* ***************************** END OF FILE ******************************* */
