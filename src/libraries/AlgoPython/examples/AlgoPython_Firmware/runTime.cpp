/* Includes **************************************************************** */
#include "systim.h"
#include "runTime.h"
#include "motor.h"
#include "sequences.h"
#include "comHandler.h"
#include "soundPlayer.h"
#include "sensor.h"
#include "led.h"
#include "thread.h"
#include "system.h"
#include "config.h"
/* Private constants ******************************************************* */

extern ComHandler comHandler;
/* Private macros ********************************************************** */

/* Private types *********************************************************** */

/* Private variables ******************************************************* */
boolean changeSpeed = 1; 

/* Private function prototypes ********************************************* */

uint32_t timer;
/* Exported functions ****************************************************** */
void processThreads() {
    /* If the thread is running:
     *  1) Process the script rows of this thread.
     *  2) When the last script row of the thread has finished, the thread stops.
     * In any case, we would want to move to the next thread to process it as well.
     */ 
    if(chk4TimeoutSYSTIM(timer,1) != SYSTIM_TIMEOUT)
    {
        return;
    }
    timer = getSYSTIM();
    if(threadArray[currentThread].isRunning) 
    {
        uint8_t run_flag = 10;
        // uint16_t scriptRowId = threadArray[currentThread].scriptRowId;
        while(run_flag--)
        {
            processScriptRow(threadArray[currentThread].scriptRowId);
            static  uint8_t prev_type = 0;
            if(prev_type != scriptRowArray[threadArray[currentThread].scriptRowId].type)
            {
                prev_type = scriptRowArray[threadArray[currentThread].scriptRowId].type;
            }
            static  uint8_t prev_thread = 0;
            if(prev_thread != currentThread)
            {
                prev_thread = currentThread;
            }

            if(((scriptRowArray[threadArray[currentThread].scriptRowId].type != TYPE_START_LOOP) && (scriptRowArray[threadArray[currentThread].scriptRowId].type != TYPE_END_LOOP))
                    || (threadArray[currentThread].isRunning == 0))
            {
                run_flag = 0;
            }

        }
    }
    // Moving to the next thread in the array.
    getNextThread();
}

void processScriptRow(byte scriptRowId) 
{
    /* 1) Process the script row by its type.
     * 2) If the script row is done, get the next script row in the current thread.
     */

    boolean isScriptRowDone = false;
    switch (scriptRowArray[scriptRowId].type) 
    {
        case TYPE_START_THREAD:
        {
            isScriptRowDone = processStartThread(scriptRowId);
            break;
        }
        case TYPE_START_LOOP:
        {
            isScriptRowDone = processStartLoop(scriptRowId);
            break;
        }
        case TYPE_END_LOOP:
        {
            isScriptRowDone = processEndLoop(scriptRowId);
            break;
        }
    }
    if(isScriptRowDone) 
    {
        getNextScriptRow(scriptRowId);
    }
}

void getNextThread() {
  currentThread = (currentThread + 1) % NUM_OF_THREADS;
}

void getNextScriptRow(byte scriptRowId) {
  
  // Reset the startTime of the last script row (in case of loops)
  // This is not done to TYPE_START_LOOP because they behave diffrently than any other TYPE
  if(scriptRowArray[scriptRowId].type != TYPE_START_LOOP) {
    scriptRowArray[scriptRowId].startTime = -1;
  }
  // Iterate over the script row array to find the next script row of the current thread.
  // NOTICE: We start the iteration from the current scriptRowId+1 in order to not repeat the same row that just finished.
  for (byte i = scriptRowId + 1; i < totalRowsInData; i++) 
  {
    if(scriptRowArray[i].type != 0 && scriptRowArray[i].threadId == currentThread) 
    {
      threadArray[currentThread].scriptRowId = i;
      return;
    }
  }
  // If we didn't find the next script row for the current thread, hence the thread is done and needs to stop.
  threadArray[currentThread].stop();
}

boolean processStartThread(byte scriptRowId) {
  /* Start Thread row will open up to 4 new threads.
   * Threads that are opened within a loop, need to acknowledge the open loop row they belong to.
   */
  for(int i = 0; i < 4; i++) {
    // Loop over the possible threads
    // dataBytes[0-3] holds the newThreadId
    byte newThreadId = scriptRowArray[scriptRowId].dataBytes[i];
    // If the nextThreadId is not 0, there is a new thread to open.
    if(newThreadId) {
      // If the current thread has opened some loop before --> the new thread acknowledge that loop (it belongs to it).
      if(threadArray[currentThread].loopRowIdThatOpenedInThisThread) {
        threadArray[newThreadId].threadBelongsToLoopRowId = threadArray[currentThread].loopRowIdThatOpenedInThisThread;
      }
      threadArray[newThreadId].start();
      assignScriptRow(newThreadId);
    }
  }
  return true;
}

boolean processStartLoop(byte scriptRowId) {
  // Script row is yet to be done by default unless specified otherwise onwards in this function.
  boolean isScriptRowDone = false;
  // Process the dataBytes array of the script row.
  byte iterations = scriptRowArray[scriptRowId].dataBytes[0]; 
  boolean isRandom = scriptRowArray[scriptRowId].dataBytes[1] && B00000001;
  boolean isInfinity = scriptRowArray[scriptRowId].dataBytes[1] & B00001000;
  
  startScriptRow(scriptRowId);
  
  // If the script row hasn't started yet --> We havn't set any iterations to the start loop.
  // isFirstIterationOfTheLoop : scriptRowArray[scriptRowId].counter;
  if(scriptRowArray[scriptRowId].counter == 0) {
    if(isRandom) {
      // If iterations specified --> Random 1-iterations Iterations.
      if(iterations) {
        iterations = random(1, iterations);
      } else {
        iterations = random(1, 11);
      }
    }
    if(iterations == 0) {
      goToEndLoop();
    }
    else {
      scriptRowArray[scriptRowId].duration = iterations-1;
      scriptRowArray[scriptRowId].startTime = iterations-1;
      scriptRowArray[scriptRowId].counter = 5;
    }
    if(isInfinity) {
      scriptRowArray[scriptRowId].duration = -1;
      scriptRowArray[scriptRowId].startTime = 255;
      scriptRowArray[scriptRowId].counter = 5;
    }
    // Let the thread acknowledge this new open loop.
    threadArray[currentThread].loopRowIdThatOpenedInThisThread = scriptRowId;
  }

  finishedScriptRow(scriptRowId);
  isScriptRowDone = true; 
  return isScriptRowDone;
}

boolean processEndLoop(byte scriptRowId) {
  // Script row is yet to be done by default unless specified otherwise onwards in this function.
  boolean isScriptRowDone = false;

  // Process the dataBytes array of the script row.
  byte startLoopScriptRowId = scriptRowArray[scriptRowId].dataBytes[0];
  byte startLoopThreadId = scriptRowArray[scriptRowId].dataBytes[1];

  // The current thread now belongs to the start loop of this end loop.
  threadArray[currentThread].threadBelongsToLoopRowId = startLoopScriptRowId;
  
  // If all the threads and all the loops inside this loop is done, we can move to the next iteration
 
  if(isAllowedToContinue(startLoopScriptRowId, currentThread)) {
    // If the duration has yet reached 0, the loop isn't done.
    if(scriptRowArray[startLoopScriptRowId].duration != 0) {
      // If Loop isn't infinity --> Move to the next iteration
      if(scriptRowArray[startLoopScriptRowId].duration != -1) {
        scriptRowArray[startLoopScriptRowId].duration--;
      }
      // GOTO start loop
      if(currentThread != startLoopThreadId) {
        threadArray[startLoopThreadId].scriptRowId = startLoopScriptRowId;  // Set the scriptRowId that the thread of the startLoopThreadId runs, to the start loop.
        threadArray[startLoopThreadId].start();                             // Start the thread of the start loop.
        threadArray[currentThread].stop();                                  // Stop the currentThread
      } else {
        threadArray[startLoopThreadId].scriptRowId = startLoopScriptRowId;  // Set the scriptRowId that the thread of the startLoopThreadId runs, to the start loop.
      }
    } else {
      debugRUN(F("Loop instruction is completed\r\n "));
      // Loop is done
      // Sometimes we can have a loop inside a loop, so we must reset the open loop counter for these cases.
      scriptRowArray[startLoopScriptRowId].counter = 0;
      isScriptRowDone = true;
    }
  }
  return isScriptRowDone;
}

//ALGOPYTHON SECTION------------------------------------------------------------------------------------------------
bool motorBusy[NUM_OF_MOTORS] = {false, false, false};
bool ledBusy[NUM_OF_LEDS] = {false,false};
bool soundBusy[SOUND_STATE_PIN] = {false};
bool sensorBusy[NUM_OF_SENSORS] = {false, false};
uint8_t sensorValue[NUM_OF_SENSORS] = {0, 0};

enum SoundState 
{
    SOUND_IDLE,
    SOUND_RUNNING,
    SOUND_DONE
};

enum LightState 
{
    LIGHT_IDLE,
    LIGHT_RUNNING,
    LIGHT_DONE
};

enum MotorFSMState 
{
    MOTOR_IDLE,
    MOTOR_RUNNING,
    MOTOR_DONE
};
enum SensorFSMState
{
    SENSOR_IDLE,
    SENSOR_RUNNING,
    SENSOR_INIT,
};

struct MotorCommand 
{
    uint32_t timer;
    uint32_t duration;
    uint8_t isActive;
    uint8_t state;
    uint8_t type;
};

MotorCommand motorCmd[3];

struct LightCommand
{
    uint32_t timer;
    uint32_t duration;
    uint8_t isActive;
    uint8_t state;
    uint8_t type;
};

LightCommand lightCmd[2];

struct SoundCommand{
    uint32_t timer;
    uint8_t Volume;
    uint8_t isActive;
    uint8_t state;
};
SoundCommand soundCmd;

struct SensorCommand{
  uint8_t isActive;
  uint8_t state;
  uint8_t min;
  uint8_t max;
  uint8_t value;
  uint8_t masterUpdateFlag;
};
SensorCommand sensorCmd[2];


void sensor_fsm()
{
    uint8_t k = 0;
    for( k = 0; k < NUM_OF_SENSORS ; k++)
    {
        uint8_t sensor_type;
        sensorCmd[k].value = sensors[k].getValue(&sensor_type);

        switch(sensorCmd[k].state)
        {
          case(SENSOR_IDLE):
          {
            break;
          }
          case(SENSOR_INIT):
          {
            if(sensorCmd[k].masterUpdateFlag == 0)
            {
                sensorCmd[k].state = SENSOR_RUNNING;
            }
            break;
          }
          case(SENSOR_RUNNING):
          {
              uint8_t value = sensorCmd[k].value;
              if((value >=  sensorCmd[k].min) && (value < sensorCmd[k].max))
              {
                sensorCmd[k].isActive = false;
                sensorCmd[k].state = SENSOR_IDLE;
              }
          }
            break;
        }
    }
}

void sound_fsm_update() 
{
    switch(soundCmd.state)
    {
        case(SOUND_IDLE):
        {

            break;
        }
        case(SOUND_RUNNING):
        {
           if (!soundPlayer.isPlaying()) 
           {
            soundCmd.isActive = false;
            soundCmd.state = SOUND_IDLE;
            } 
            break;
        }
    }
}

void light_fsm_update() 
{
    uint8_t k = 0;
    for( k = 0; k < NUM_OF_LEDS ; k++)
    {
        switch(lightCmd[k].state)
        {
            case (LIGHT_IDLE):
            {
                break;   
            }
            case (LIGHT_RUNNING):
            {
                if(lightCmd[k].type == 0)
                {
                    if(chk4TimeoutSYSTIM(lightCmd[k].timer,lightCmd[k].duration) == SYSTIM_TIMEOUT)
                    {
                        leds[k].stop(0);
                        lightCmd[k].isActive = false;
                        lightCmd[k].state = LIGHT_IDLE;
                    }
                }
                break;   
            }
            case (LIGHT_DONE):
            {

                break;
            }
        }
    }
}

uint8_t cmd_process_light(uint8_t led_port, uint8_t led_type,float led_duration, uint8_t led_power, uint8_t led_r, uint8_t led_g, uint8_t led_b) 
{
    uint8_t led_active_flag[2];
    led_active_flag[0] = led_port & 0x01;
    led_active_flag[1] = led_port & 0x02;
    uint8_t k = 0;
    for(k = 0; k < NUM_OF_MOTORS; k++)
    {
        if(led_active_flag[k])
        {
            if(lightCmd[k].state != LIGHT_IDLE)
            {
                leds[k].stop(0);
            }
            lightCmd[k].state = LIGHT_RUNNING;
            lightCmd[k].type = led_type;
            lightCmd[k].duration = led_duration * 1000;
            lightCmd[k].timer = millis();
            lightCmd[k].isActive = true;
            leds[k].start(led_r, led_g, led_b,0);      
        }
    }
    return true;
}

uint8_t cmd_process_playSound(uint8_t sound_id, uint8_t volume) {

    if (sound_id > 15) return false;
    if(soundCmd.state != SOUND_IDLE)
    {
        soundPlayer.stop();
    }
    soundPlayer.setVolume(volume);
    soundCmd.timer = getSYSTIM();
    soundCmd.state = SOUND_RUNNING;  
    soundCmd.isActive = true;
 
    soundPlayer.play(sound_id, 0xFF);
    return true;

}

void motor_fsm_update() {
    uint8_t k = 0;
    for( k = 0; k < NUM_OF_MOTORS; k++)
    {
        switch(motorCmd[k].state)
        {
            case (MOTOR_IDLE):
            {
                break;   
            }
            case (MOTOR_RUNNING):
            {
                if(motorCmd[k].type == 0)
                {
                    if(chk4TimeoutSYSTIM(motorCmd[k].timer,motorCmd[k].duration) == SYSTIM_TIMEOUT)
                    {
                        motors[k].stop(0);
                        motorCmd[k].isActive = false;
                        motorCmd[k].state = MOTOR_IDLE;
                    }
                }
                break;   
            }
            case (MOTOR_DONE):
            {

                break;
            }

        }
}
}
uint8_t cmd_process_move(uint8_t motor_port,uint8_t motor_type,float motor_duration ,uint8_t motor_power,uint8_t motor_dir) 
{
    uint8_t motor_active_flag[3];
    motor_active_flag[0] = motor_port & 0x01;
    motor_active_flag[1] = motor_port & 0x02;
    motor_active_flag[2] = motor_port & 0x04;
    uint8_t k = 0;
    for(k = 0; k < NUM_OF_MOTORS; k++)
    {
        if(motor_active_flag[k])
        {
          if(motorCmd[k].state != MOTOR_IDLE)
          {
            motors[k].stop(0);
          }
          motorCmd[k].state = MOTOR_RUNNING;
          motorCmd[k].type = motor_type;
          motorCmd[k].duration = motor_duration * 1000;              
          motorCmd[k].timer = millis();
          motorCmd[k].isActive = true;
          motors[k].start(motor_power, motor_dir == 1 ? 1 : 0, 0);      
        }
    }
    return true;
}
uint8_t cmd_process_wait_sensor (uint8_t sensor_port, uint8_t min, uint8_t max)
{
    uint8_t sensor_active_flag[2];
    sensor_active_flag[0] = sensor_port & 0x01;
    sensor_active_flag[1] = sensor_port & 0x02;
    uint8_t k = 0;
    for(k = 0; k < NUM_OF_SENSORS; k++)
    {
        if(sensor_active_flag[k])
        {
          sensorCmd[k].min = min ;
          sensorCmd[k].max = max ;
          sensorCmd[k].state = SENSOR_INIT;
          sensorCmd[k].isActive = true;
          sensorCmd[k].masterUpdateFlag = 1;
        }
    }
}
uint8_t cmd_process_move_stop(uint8_t motor_port)
{
    uint8_t motor_active_flag[3];
    motor_active_flag[0] = motor_port & 0x01;
    motor_active_flag[1] = motor_port & 0x02;
    motor_active_flag[2] = motor_port & 0x04;
    uint8_t k = 0;
    for(k = 0; k < NUM_OF_MOTORS; k++)
    {
        if(motor_active_flag[k])
        {
          motorCmd[k].state = MOTOR_DONE;
          motorCmd[k].isActive = false;
          motors[k].stop(0);
        }
    }
}
uint8_t cmd_process_light_stop(uint8_t light_port)
{
  if(light_port < 1 && light_port > 2)
  {
    return false;
  }
  else
  {
    if(light_port == 1)
    {
      leds[0].stop(0);
      lightCmd[0].isActive = false;
      lightCmd[0].state = LIGHT_DONE;
    }
    else if(light_port == 2)
    {
      leds[1].stop(0);
      lightCmd[1].isActive = false;
      lightCmd[1].state = LIGHT_DONE;
    }
  } 
}
uint8_t cmd_process_sound_stop()
{
  soundCmd.isActive = false;
  soundCmd.state = SOUND_DONE;
  soundPlayer.stop();
}

void motorAndDirectionHelper(byte directionDataByte, byte *motorAndDirection) {
  // isMotorX - The direction of the motor --> CW = 0 | CCW = 1 | Not Selected = 255;
  
  // Set motorA
  if((directionDataByte & B00000011) == B00000011) {
    motorAndDirection[0] = CCW;
  } else if ((directionDataByte & B00000011) == B00000010) {
    motorAndDirection[0] = CW;
  }
  // Set motorB
  if((directionDataByte & B00001100) == B00001100) {
    motorAndDirection[1] = CCW;
  } else if ((directionDataByte & B00001100) == B00001000) {
    motorAndDirection[1] = CW;
  }
  // Set motorC
  if((directionDataByte & B00110000) == B00110000) {
    motorAndDirection[2] = CCW;
  } else if ((directionDataByte & B00110000) == B00100000) {
    motorAndDirection[2] = CW;
  }
}

//byte motorPowerHelper(byte powerDataByte, int encA,int encB,byte motor) {
byte motorPowerHelper(byte powerDataByte) {
  byte powerLevel = 0;
  if(powerDataByte == B00100000) {
    powerLevel = 1;
  } else if(powerDataByte == B01000000) {
    powerLevel = 2;
    // return 255;
  } else if(powerDataByte == B10000000) {
    powerLevel = 3;
  }
  if (powerDataByte == B00000000) {
    return 0;
    }
    else{
    // Mapping the power of 0-3 to a PWM Value of 0-255
    // return map(powerLevel, 0, 3, 0, 255);     // Values would be: LOW = 85, MED = 170, HIGH = 255
    // return map(powerLevel, 1, 3, 100, 255);   // Values would be: LOW = 100, MED = 177.5, HIGH = 255
    return powerLevel = map(powerLevel, 1, 3, 145, 255);      // Values would be: LOW = 145, MED = 200, HIGH = 255
    // return powerLevel = map(powerLevel, 1, 3, 145, 200);      // Values would be: LOW = 145, MED = 200, HIGH = 255
  }
 
}
void algopython_fsm_update() 
{
    motor_fsm_update();
    light_fsm_update();
    sound_fsm_update();
    sensor_fsm();
}

void fillStatusPayload(uint8_t* buf) 
{
    buf[0] = motorCmd[0].isActive ;
    buf[1] = motorCmd[1].isActive;
    buf[2] = motorCmd[2].isActive;
    buf[3] = lightCmd[0].isActive;
    buf[4] = lightCmd[1].isActive;
    buf[5] = soundCmd.isActive;
    buf[6] = sensorCmd[0].isActive;
    buf[7] = sensorCmd[1].isActive;
    buf[8] = sensorCmd[0].value;
    buf[9] = sensorCmd[1].value;
    sensorCmd[0].masterUpdateFlag = 0;
    sensorCmd[1].masterUpdateFlag = 0;
}
//ALGOPYTHON SECTION------------------------------------------------------------------------------------------------

/* Private functions ******************************************************* */

/* ***************************** END OF FILE ******************************* */
