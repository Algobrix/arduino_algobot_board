/* Includes **************************************************************** */
#include "motorEncoders.h"
#include "motor.h"

/* Private constants ******************************************************* */


/* Private macros ********************************************************** */

/* Private types *********************************************************** */

/* Private variables ******************************************************* */
byte scriptRowIdEncoderA = 0;
byte scriptRowIdEncoderB = 0;
byte scriptRowIdEncoderC = 0;

//int mapStep = 1555; // snail transmission 3 PPR motor
//int tenDegrees = 157; // sets the rotations 3 PPR motor
int mapStep = 3050; // snail transmission 6 PPR motor
//int mapStep = 4550; // snail transmission 6 PPR motor 20 cm
int tenDegrees = 295; // sets the rotations 6 PPR motor
int diff = 0;
int preDiff = 0; // parameter for PD control

byte tempPowerA = 0; 
byte tempPowerB = 0;
int maxPower = 215; 
int minPower = 121; 
int stepPow = 10; // power step size
unsigned long motorBalanceTimer;
int encoderDuration = 65535; // Maximum by default
// For custom PWM of motors A and B :  
boolean isHighA = false;
boolean isHighB = false;

// *********************************************************


/* Private function prototypes ********************************************* */

/* Exported functions ****************************************************** */
void encodersInit() { 
  pinMode(MOTOR_A_ENCODER, INPUT);
  pinMode(MOTOR_B_ENCODER, INPUT);
  pinMode(MOTOR_C_ENCODER, INPUT);
  // When Changing Bits -> Use mask so that only the significant bits are affected

  // MOTOR A - Initialize Timer3
  TCNT3 = 0;                                  // Timer/Counter Register. The actual timer value is stored here.
  TIFR3 = 0;                                  // Timer/Counter Interrupt Flag Register
  TCCR3A = 0;                                 // PWM Mode is OFF.
  TCCR3B = (TCCR3B & B11110001) | B00001110;  // External clock source on T3 pin. Clock on falling edge. + CTC = Clear Timer (on) Compare Mode.
  OCR3A = 0;                                  // The Compare value of Timer4 --> If TCNT4 == OCR3A --> Call Interrupt.
  TIMSK3 = (TIMSK3 & B11111101) | B00000010;  // Enable Timer3 compare interrupt - ALWAYS ENABLED.
  
  // MOTOR B - Initialize Timer1
  TCNT1 = 0;
  TIFR1 = 0;
  TCCR1A = 0;
  TCCR1B = (TCCR1B & B11110001) | B00001110;
  OCR1A = 0;
  TIMSK1 = (TIMSK1 & B11111101) | B00000010;
  
  // MOTOR C - Initialize Timer4
  TCNT4 = 0;
  TIFR4 = 0;
  TCCR4A = 0;
  TCCR4B = (TCCR4B & B11110001) | B00001110;
  OCR4A = 0;
  TIMSK4 = (TIMSK4 & B11111101) | B00000010;

  // Initialize TIMER2 for PWM of motors A and B.
  TCNT2 = 0;
  TIFR2 = 0;
  TCCR2A = 0;
  TCCR2B = (TCCR2B & B11111100) | B00000011;  // Clock I/O with prescaler of 64.
  OCR2A = 0;                                  // Compare Value A - for motor A
  OCR2B = 0;                                  // Compare Value B - for motor B
  TIMSK2 = (TIMSK2 & B11111111) | B00000000;  // Timer2 disabled by default - enabled when used for enabling the motors.
}

void initOneEncoder(int motorId) {
  pinMode(MOTOR_A_ENCODER, INPUT);
  pinMode(MOTOR_B_ENCODER, INPUT);
  pinMode(MOTOR_C_ENCODER, INPUT);
  // When Changing Bits -> Use mask so that only the significant bits are affected
  switch(motorId) {
    case 0:
      // MOTOR A - Initialize Timer3
      TCNT3 = 0;                                  // Timer/Counter Register. The actual timer value is stored here.
      TIFR3 = 0;                                  // Timer/Counter Interrupt Flag Register
      TCCR3A = 0;                                 // PWM Mode is OFF.
      TCCR3B = (TCCR3B & B11110001) | B00001110;  // External clock source on T3 pin. Clock on falling edge. + CTC = Clear Timer (on) Compare Mode.
      OCR3A = 0;                                  // The Compare value of Timer4 --> If TCNT4 == OCR3A --> Call Interrupt.
      TIMSK3 = (TIMSK3 & B11111101) | B00000010;  // Enable Timer3 compare interrupt - ALWAYS ENABLED.
      break;
      // MOTOR B - Initialize Timer1
    case 1:
      TCNT1 = 0;
      TIFR1 = 0;
      TCCR1A = 0;
      TCCR1B = (TCCR1B & B11110001) | B00001110;
      OCR1A = 0;
      TIMSK1 = (TIMSK1 & B11111101) | B00000010;
      break;
    case 2:  
      // MOTOR C - Initialize Timer4
      TCNT4 = 0;
      TIFR4 = 0;
      TCCR4A = 0;
      TCCR4B = (TCCR4B & B11110001) | B00001110;
      OCR4A = 0;
      TIMSK4 = (TIMSK4 & B11111101) | B00000010;
      break;
  }
}

void setMotorInterrupt(byte motorId, long timeToInterruprt, byte scriptRowId) {
  switch(motorId) {
    case 0:
      scriptRowIdEncoderA = scriptRowId;
      TCNT3 = 0;
      TIFR3 = 0;
      if(timeToInterruprt == -1) TIMSK3 = (TIMSK3 & B11111101);
      else OCR3A = (uint16_t)timeToInterruprt;
      break;
    case 1:
      scriptRowIdEncoderB = scriptRowId;
      TCNT1 = 0;
      TIFR1 = 0;
      if(timeToInterruprt == -1) TIMSK1 = (TIMSK1 & B11111101);
      else OCR1A = (uint16_t)timeToInterruprt;
      break;
    case 2:
      scriptRowIdEncoderC = scriptRowId;
      TCNT4 = 0;
      TIFR4 = 0;
      if(timeToInterruprt == -1) TIMSK4 = (TIMSK4 & B11111101);
      else OCR4A = (uint16_t)timeToInterruprt;
      break;
  }
}

void checkEncodersOfMotor(byte motorId) 
{
    boolean isMotorARunning = motors[0].isRunning();
    boolean isMotorBRunning = motors[1].isRunning();
    boolean isMotorCRunning = motors[2].isRunning();
    debugENCODER(F("Motor ID is:   "));
    debugENCODER(motorId);
    debugENCODER(F("\r\n"));
    switch(motorId) 
    {
        case 0: 
        {
            byte scriptRowId = scriptRowIdEncoderA;
            if(scriptRowId != scriptRowIdEncoderB && scriptRowId != scriptRowIdEncoderC) 
            {
                scriptRowArray[scriptRowId].duration = 0;
            } 
            else if(scriptRowId != scriptRowIdEncoderB && !isMotorCRunning) 
            {
                scriptRowArray[scriptRowId].duration = 0;
            } 
            else if(scriptRowId != scriptRowIdEncoderC && !isMotorBRunning) 
            {
                scriptRowArray[scriptRowId].duration = 0;
            } 
            else if(!isMotorBRunning && !isMotorCRunning) 
            {
                scriptRowArray[scriptRowId].duration = 0;
            }
            break;
        }
        case 1: 
        {
            byte scriptRowId = scriptRowIdEncoderB;
            if(scriptRowId != scriptRowIdEncoderA && scriptRowId != scriptRowIdEncoderC) 
            {
                scriptRowArray[scriptRowId].duration = 0;
            } 
            else if(scriptRowId != scriptRowIdEncoderA && !isMotorCRunning) 
            {
                scriptRowArray[scriptRowId].duration = 0;
            }
            else if(scriptRowId != scriptRowIdEncoderC && !isMotorARunning) 
            {
                scriptRowArray[scriptRowId].duration = 0;
            } 
            else if(!isMotorARunning && !isMotorCRunning) 
            {
                scriptRowArray[scriptRowId].duration = 0;
            }
            break;
        }
        case 2: 
        {
            byte scriptRowId = scriptRowIdEncoderC;
            if(scriptRowId != scriptRowIdEncoderA && scriptRowId != scriptRowIdEncoderB) 
            {
                scriptRowArray[scriptRowId].duration = 0;
            } 
            else if(scriptRowId != scriptRowIdEncoderA && !isMotorBRunning) 
            {
                scriptRowArray[scriptRowId].duration = 0;
            }
            else if(scriptRowId != scriptRowIdEncoderB && !isMotorARunning) 
            {
                scriptRowArray[scriptRowId].duration = 0;
            } 
            else if(!isMotorARunning && !isMotorBRunning) 
            {
                scriptRowArray[scriptRowId].duration = 0;
            }
            break;
        }
    }
}

void myAnalogWrite(byte pin, byte val) 
{
    switch(pin) 
    {
        case MOTOR_A_PWM: 
        {
            switch (val) 
            {
                case 0:
                case 255: 
                {
                    isHighA = false;  // Disable flag
                    TIMSK2 = (TIMSK2 & B11111101) | B00000000; // Disable Compare A Interrupt
                    digitalWrite(pin, val);
                    break;
                }
                default: 
                {
                    OCR2A = motors[0].pwm;
                    TIMSK2 = (TIMSK2 & B11111101) | B00000010; // Enable Compare A Interrupt
                    break;
                }
            }
            break;
        }
        case MOTOR_B_PWM: 
        {
            switch(val) 
            {
                case 0:
                case 255: 
                {
                    isHighB = false;  // Disable flag
                    TIMSK2 = (TIMSK2 & B11111011) | B00000000; // Disable Compare B Interrupt
                    digitalWrite(pin, val);
                    break;
                }
                default: 
                {
                    OCR2B = motors[1].pwm;
                    TIMSK2 = (TIMSK2 & B11111011) | B00000100; // Enable Compare B Interrupt
                    break;
                }
            }
            break;
        }
        default: 
        {
            analogWrite(pin, val);
            break;
        }
    }
}

// Timer compare interrupt service routine
/*ISR(TIMER3_COMPA_vect) {
  noInterrupts();
  if (scriptRowIdEncoderA == scriptRowIdEncoderB){ // if both motors came from the same cube, and one finished, stop both together
    motors[0].stop(scriptRowIdEncoderA); // Stop motor A
    motors[1].stop(scriptRowIdEncoderB); // Stop motor B
    TIMSK2 = (TIMSK2 & B11111101) | B00000000; // Disable Compare A Interrupt
    // initialize B , as we are inside timer 3 that sets encoder A and we want to stop B as well. 
    initOneEncoder(1); 
    //debugENCODERln(F("TIMER3 - MOTOR A: "));
  }
  checkEncodersOfMotor(0); // check encoder A
  interrupts();
}


ISR(TIMER1_COMPA_vect) {
  noInterrupts();
  if (scriptRowIdEncoderA == scriptRowIdEncoderB){ // if both motors came from the same cube, and one finished, stop both together
    motors[0].stop(scriptRowIdEncoderA); // Stop motor A
    motors[1].stop(scriptRowIdEncoderB); // Stop motor B
    TIMSK2 = (TIMSK2 & B11111011) | B00000000; // Disable Compare B Interrupt   
    // initialize A, as we are inside timer 1 that sets encoder B and we want to stop motor A as well. 
    initOneEncoder(0); 
  }
  checkEncodersOfMotor(1);
  interrupts();
}


*/
ISR(TIMER3_COMPA_vect) 
{
  noInterrupts();
  motors[0].stop(scriptRowIdEncoderA);
//debugENCODER(F("ISR: 0, stop A\r\n"));
  checkEncodersOfMotor(0);
  interrupts();
}

ISR(TIMER1_COMPA_vect) 
{
  noInterrupts();
  motors[1].stop(scriptRowIdEncoderB);
//debugENCODER(F("ISR: 1, stop B\r\n"));
  checkEncodersOfMotor(1);
  interrupts();
}

ISR(TIMER4_COMPA_vect) 
{
  noInterrupts();
  motors[2].stop(scriptRowIdEncoderC);
//debugENCODER(F("ISR: 4, stop C\r\n"));
  checkEncodersOfMotor(2);
  interrupts();
}

ISR(TIMER2_COMPA_vect) 
{
  noInterrupts();
  if(isHighA) 
  {
    digitalWrite(MOTOR_A_PWM, LOW);
    OCR2A = 0;
    isHighA = false;
  } 
  else 
  {
    digitalWrite(MOTOR_A_PWM, HIGH);
    OCR2A = motors[0].pwm;
    isHighA = true;
  }
  interrupts();
}

ISR(TIMER2_COMPB_vect) 
{
  noInterrupts();
  if(isHighB) 
  {
    digitalWrite(MOTOR_B_PWM, LOW);
    OCR2B = 0;
    isHighB = false;
  } 
  else 
  {
    digitalWrite(MOTOR_B_PWM, HIGH);
    OCR2B = motors[1].pwm;
    isHighB = true;
  }
  interrupts();
}

boolean motorBalanceSpeed()
{
  // This function balances the speed between motors A and B in case of driving action
  if ( ( diff > 10) ) 
  {
    if (diff > preDiff) 
    { // this means the difference is now bigger and we need to change the speed
      debugENCODER(F("diff >preDiff"));
      preDiff = diff;
      if ( tempPowerB + stepPow + diff/3 >= maxPower)  
      { // this means we can't make B faster, so we slow down A
          tempPowerA = tempPowerA - stepPow - diff/3; // slowing down motor A - We use temp to keep "power" fixed on 255. 
      }
      else 
      { // make B faster 
        tempPowerB = tempPowerB + stepPow + diff/3; // make B faster          
      }
      return 1;    
    }
  } 
  else 
  {
      if ( ( diff < -10)  ) 
      {
        if (diff < preDiff) 
        { // this means the difference is now bigger
            //debugENCODERln(F("diff < preDiff"));
            preDiff = diff ;
            // Notice diff is negative
            if (tempPowerA + stepPow - diff/3>= maxPower)  
            { // this means we can't make A faster, so we slow down B
              tempPowerB = tempPowerB - stepPow + diff/3; // slowing down motor B, notice diff is negative
            }
            else 
            { // make A faster 
              tempPowerA = tempPowerA + stepPow - diff/3; // make A faster, notice diff is negative
            }
        }
        return 1;    
      }
  }
  return 0;  
}


/* Private functions ******************************************************* */

/* ***************************** END OF FILE ******************************* */
