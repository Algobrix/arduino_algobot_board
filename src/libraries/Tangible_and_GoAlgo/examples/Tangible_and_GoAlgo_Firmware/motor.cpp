/* Includes **************************************************************** */
#include "motor.h"
#include "comHandler.h"

/* Private constants ******************************************************* */


/* Private macros ********************************************************** */

/* Private types *********************************************************** */

/* Private variables ******************************************************* */
Motor motors[NUM_OF_MOTORS] = 
{
  Motor(MOTOR_A_DIR, MOTOR_A_PWM),
  Motor(MOTOR_B_DIR, MOTOR_B_PWM),
  Motor(MOTOR_C_DIR, MOTOR_C_PWM)
};



/* Private function prototypes ********************************************* */

/* Exported functions ****************************************************** */

/* Private functions ******************************************************* */
Motor::Motor(byte dirPin, byte pwmPin) 
{
    this->dirPin = dirPin;
    this->pwmPin = pwmPin;
}

boolean Motor::isRunning() 
{
    if(pwm > 0) 
    {
        return true;
    }
    return false;
}

void Motor::start(byte pwm, boolean dir, byte scriptRowId) 
{
    if(this->scriptRowId != 0) 
    {
        prematureEndMotorScript(this->scriptRowId);
    }
    this->pwm = pwm;
    this->direction = dir;
    this->scriptRowId = scriptRowId;
    digitalWrite(dirPin, dir);
    myAnalogWrite(pwmPin, pwm);
}

void Motor::changeSpeed(byte pwm) 
{

    this->pwm = pwm;
    myAnalogWrite(pwmPin, pwm);
}

void Motor::stop(byte scriptRowId) 
{
    // If the motor belongs to the the scriptRowId that called to stop.
    if(this->scriptRowId == scriptRowId) 
    {
        this->scriptRowId = 0;
        this->pwm = 0;
        myAnalogWrite(pwmPin, 0);
        debugMOTOR(F("Stopping the motor on the scriptRowId ["));
        debugMOTOR(scriptRowId); 
        debugMOTOR(F("\r\n"));
    }
}

void motorsInit() 
{
  for(int i = 0; i < NUM_OF_MOTORS; i++) 
  {
    motors[i].stop(motors[i].scriptRowId);
  }
}

void prematureEndMotorScript(byte scriptRowId) 
{
  for(int i = 0; i < NUM_OF_MOTORS; i++) 
  {
    if(motors[i].scriptRowId == scriptRowId) 
    {
      motors[i].stop(scriptRowId);
    }
  }
  // If infinity, the script row stopped and is not running anymore.
  if(scriptRowArray[scriptRowId].duration == -1) 
  {
    numOfRunningScriptRows--;
  } 
  // Else, the scriptRow duration goes down to 0 for it to finish and move to the next script row in the thread.
  else 
  {
    scriptRowArray[scriptRowId].duration = 0;
  }
  comHandler.sendRowExecute(OP_COMMAND_ROW_EXECUTION_PREMATURE_END, scriptRowId);
}


/* ***************************** END OF FILE ******************************* */
