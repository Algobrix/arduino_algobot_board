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
        case TYPE_MOVE_MOTOR:
        {
            isScriptRowDone = processMoveMotor(scriptRowId);
            break;
        }
        case TYPE_LED:
        {
            isScriptRowDone = processLed(scriptRowId);
            break;
        }
        case TYPE_STOP:
        {
            isScriptRowDone = processStop(scriptRowId);
            break;
        }
        case TYPE_WAIT_SENSOR:
        {
            isScriptRowDone = processWaitSensor(scriptRowId);
            break;
        }
        case TYPE_WAIT:
        {
            isScriptRowDone = processWait(scriptRowId);
            break;
        }
        case TYPE_SOUND:
        {
            isScriptRowDone = processSound(scriptRowId);
            break;
        }
        case TYPE_PROGRAM:
        {
            isScriptRowDone = processSequence(scriptRowId);
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

boolean processLed(byte scriptRowId) {
  // Script row is yet to be done by default unless specified otherwise onwards in this function.
  boolean isScriptRowDone = false;
  // Process the dataBytes array of the script row.
  boolean isLedSelected[NUM_OF_LEDS] = { scriptRowArray[scriptRowId].dataBytes[0] & B01000000, scriptRowArray[scriptRowId].dataBytes[0] & B10000000 };
  byte duration = scriptRowArray[scriptRowId].dataBytes[1];
  boolean isInfinity = scriptRowArray[scriptRowId].dataBytes[2] & B00001000;
  byte r = scriptRowArray[scriptRowId].dataBytes[4];
  byte g = scriptRowArray[scriptRowId].dataBytes[5];
  byte b = scriptRowArray[scriptRowId].dataBytes[6];
  boolean isRandom = scriptRowArray[scriptRowId].dataBytes[7] & B00000001;
  
  // If the script row hasn't started yet
  if (scriptRowArray[scriptRowId].startTime == -1) {
    startScriptRow(scriptRowId);
    if(isRandom) {
      // If duration is specified, random between 1 sec to the given time
      if(duration) {
        duration = random(10, 61); // Default of 1-6 seconds.
      } else {
        duration = random(10, duration);
      }
      // Pick a random color
      switch(random(1, 8)) {
        case 1:
          r = 255;
          g = 0;
          b = 0;
          break;
        case 2: // Green
          r = 0;
          g = 255;
          b = 0;
          break;
        case 3: // Blue
          r = 0;
          g = 0;
          b = 255;
          break;
        case 4: // Orange
          r = 100;
          g = 65;
          b = 0;
          break;
        case 5: // Yellow
          r = 255;
          g = 255;
          b = 0;
          break;
        case 6: // Purple
          r = 148;
          g = 0;
          b = 211;
          break;
        case 7: // White
          r = 255;
          g = 255;
          b = 255;
          break;
      }
    }
    // Check for infinity byte
    if(isInfinity) {
      scriptRowArray[scriptRowId].duration = -1;
      isScriptRowDone = true;
    } else {
      // Set the duration
      scriptRowArray[scriptRowId].duration = constrain(duration, MIN_TIME_VALUE, MAX_TIME_VALUE) * DURATION_MULTIPLIER;
    }
    // After parsing all the dataBytes --> start the LED's
    scriptRowArray[scriptRowId].startTime = getSYSTIM();
    for(int i = 0; i < NUM_OF_LEDS; i++) {
      if(isLedSelected[i]) {
        leds[i].start(r, g, b, scriptRowId);
      }
    }
  } else {
    // If not infinity and the duration time has passed
    // if(scriptRowArray[scriptRowId].duration != -1 && scriptRowArray[scriptRowId].startTime + scriptRowArray[scriptRowId].duration < millis()) {
    if((scriptRowArray[scriptRowId].duration != -1) && (chk4TimeoutSYSTIM(scriptRowArray[scriptRowId].startTime,scriptRowArray[scriptRowId].duration) == SYSTIM_TIMEOUT)) {
      for(int i = 0; i < NUM_OF_LEDS; i++) {
        if(isLedSelected[i]) {
          leds[i].stop(scriptRowId);
        }
      }
      finishedScriptRow(scriptRowId);
      isScriptRowDone = true;
    }
  }
  return isScriptRowDone;
}

boolean processStop(byte scriptRowId) {
  // Script row is yet to be done by default unless specified otherwise onwards in this function.
  boolean isScriptRowDone = false;
  // Process the dataBytes array of the script row.
  byte motorAndDirection[NUM_OF_MOTORS] = {255, 255, 255};
  byte stopMotors = scriptRowArray[scriptRowId].dataBytes[0];
  boolean stopLed[NUM_OF_LEDS] = { scriptRowArray[scriptRowId].dataBytes[1] & B01000000, scriptRowArray[scriptRowId].dataBytes[1] & B10000000 };
  byte stopSound = scriptRowArray[scriptRowId].dataBytes[2];
  byte stopPlayingScript = scriptRowArray[scriptRowId].dataBytes[3];
  
  startScriptRow(scriptRowId);
  motorAndDirectionHelper(stopMotors, motorAndDirection);
  if(stopPlayingScript) {
    stopPlaying();
  } else {
    // The stopping is done with the "start \ play" functions in order to check for premature end on the script row that will be stopped.
    // Stop Motors
    for(int i = 0; i < NUM_OF_MOTORS; i++) {
      if(motorAndDirection[i] != 255) {
        motors[i].start(0, 0, scriptRowId);
        motors[i].scriptRowId = 0;
      }
    }
    // Stop LED's
    for(int i = 0; i < NUM_OF_LEDS; i++) {
      if(stopLed[i]) {
        leds[i].start(0, 0, 0, scriptRowId);
        leds[i].scriptRowId = 0;
      }
    }
    // Stop Sound
    if(stopSound) {
      soundPlayer.play(255, scriptRowId);
    }
    isScriptRowDone = true;
  }
  finishedScriptRow(scriptRowId);
  return isScriptRowDone;
}

boolean processWaitSensor(byte scriptRowId) {
  // Script row is yet to be done by default unless specified otherwise onwards in this function.
  boolean isScriptRowDone = false;
  // Process the dataBytes array of the script row.
  boolean isSensorSelected[NUM_OF_SENSORS] = {scriptRowArray[scriptRowId].dataBytes[0] & B00010000 , scriptRowArray[scriptRowId].dataBytes[0] & B00100000};
  boolean isMatchOnTrue = scriptRowArray[scriptRowId].dataBytes[2] & B00010000;
  boolean isMatchOnFalse = scriptRowArray[scriptRowId].dataBytes[2] & B00001000;
  // The line below, takes 2 diffrent bytes that represent the match values of 0 - 10
  // and connects the bits into 1 int (instead of 2 bytes)
  // matchValues bits that we care about are  --> B00000   1  1  1  1  1  1  1  1  1  1  1
  // matchValues actual values of each bit is -->  XXXXX  10  9  8  7  6  5  4  3  2  1  0
  unsigned int matchValues = ( ((unsigned int)scriptRowArray[scriptRowId].dataBytes[1] << 3) + ((unsigned int)scriptRowArray[scriptRowId].dataBytes[2] >> 5) );
  boolean isWaitScript = scriptRowArray[scriptRowId].dataBytes[7];
  if(isWaitScript) {
    isScriptRowDone = processWait(scriptRowId);
  } else {
    // If the script row hasn't started yet
    if (scriptRowArray[scriptRowId].startTime == -1) {
      startScriptRow(scriptRowId);
      scriptRowArray[scriptRowId].startTime = getSYSTIM();
    } else {
      for(int i = 0; i < NUM_OF_SENSORS; i++) 
      {
          if(isSensorSelected[i]) 
          {
              uint8_t sensor_type = 0;
              byte sensorValue = sensors[i].getValue(&sensor_type);
              /**
               * 3 condition we check to know if the sensor value matched:
               *  1. If true AND sensor value is not 0.
               *  2. If false AND sensor value is 0.
               *  3. If sensorValue matches any of the bits of matchValue :
               *    To do so, we will "cast" the sensorValue from a byte that represent the actual number that we want to match,
               *    to an int where the matching value is the only bit that is "true".
               *      Example: sensorValue = 2, will turn into an int that looks like : 00000000 000000100
               *      When we do a bitwise AND to that value with the matchValues we will get true only if the matchValues bit of 2 is true also. 
               */
              if(sensor_type == ALGOSENSOR_TYPE_PWM)
              {
                  uint8_t isSensorValueMatched = 0;
                  if(isMatchOnTrue && sensorValue) 
                  {
                      isSensorValueMatched = 1;
                  }
                  else if (isMatchOnFalse && !sensorValue)
                  {
                      isSensorValueMatched = 1;
                  }
                  else
                  {
#if USE_CUSTOM_LEVEL_LIMITS ==1
                      if(sensorValue != 0)
                      {
                          if((matchValues == 0x6) && (sensorValue>=CUSTOM_LEVEL1_MIN) && (sensorValue <= CUSTOM_LEVEL1_MAX)) //low 
                          {
                              isSensorValueMatched = 1;
                          }
                          else if((matchValues == 0x3E) && (sensorValue>=CUSTOM_LEVEL2_MIN) && (sensorValue <= CUSTOM_LEVEL2_MAX)) // medium
                          {
                              isSensorValueMatched = 1;
                          }
                          else if((matchValues == 0x7FE) && (sensorValue>=CUSTOM_LEVEL3_MIN) && (sensorValue <= CUSTOM_LEVEL3_MAX)) // medium
                          {
                              isSensorValueMatched = 1;
                          }
                      }
#else
                       isSensorValueMatched = matchValues & ((unsigned int)1 << (unsigned int)sensorValue);
#endif
                  }
                  if(isSensorValueMatched != 0)
                  {
                      finishedScriptRow(scriptRowId);
                      isScriptRowDone = true;
                      break;
                  }
              }
              else
              {
#ifdef USE_SENSOR_RAW_VALUE
#define SENSOR_RAW_VALUE_MAX                255
#define SENSOR_RAW_VALUE_LEVEL3_LIMIT       20
#define SENSOR_RAW_VALUE_LEVEL2_LIMIT       10
#define SENSOR_RAW_VALUE_LEVEL1_LIMIT       1
                  uint8_t isSensorValueMatched  = 0;
                  switch(matchValues)
                  {
                      case(0b011111111110):
                      {
                          if((sensorValue >= SENSOR_RAW_VALUE_LEVEL3_LIMIT) && (sensorValue < SENSOR_RAW_VALUE_MAX))
                          {
                              isSensorValueMatched = 1;

                          }

                          break;
                      }
                      case(0b000000111110)
                      {
                          if((sensorValue >= SENSOR_RAW_VALUE_LEVEL2_LIMIT) && (sensorValue < SENSOR_RAW_VALUE_LEVEL3_LIMIT))
                          {
                              isSensorValueMatched = 1;
                          }
                          break;
                      }
                      case(0b000000000110)
                      {
                          if((sensorValue >= SENSOR_RAW_VALUE_LEVEL1_LIMIT) && (sensorValue < SENSOR_RAW_VALUE_LEVEL2_LIMIT))
                          {
                              isSensorValueMatched = 1;
                          }
                          break;
                      }
                      case(0b000000000000)
                      {
                          if((sensorValue >= SENSOR_RAW_VALUE_LEVEL1_LIMIT))
                          {
                              isSensorValueMatched = 1;
                          }
                          break;
                      }

                  }
                  if(isSensorValueMatched)
                  {
                      finishedScriptRow(scriptRowId);
                      isScriptRowDone = true;
                      break;
                  }
#else
                  uint8_t isSensorValueMatched  = 0;
                  switch(matchValues)
                  {
                      case(0b011111111110):
                      {
                          if((sensorValue >= 20) && (sensorValue < 255))
                          {
                              isSensorValueMatched = 1;

                          }

                          break;
                      }
                      case(0b000000111110):
                      {
                          if((sensorValue >= 10) && (sensorValue < 20))
                          {
                              isSensorValueMatched = 1;
                          }
                          break;
                      }
                      case(0b000000000110):
                      {
                          if((sensorValue >= 1) && (sensorValue < 10))
                          {
                              isSensorValueMatched = 1;
                          }
                          break;
                      }
                      case(0b000000000000):
                      {
                          if((sensorValue >= 1))
                          {
                              isSensorValueMatched = 1;
                          }
                          break;
                      }

                  }
                  if(isSensorValueMatched)
                  {
                      finishedScriptRow(scriptRowId);
                      isScriptRowDone = true;
                      break;
                  }
#endif
              }
          }
      }
    }
  }
  return isScriptRowDone;
}

boolean processWait(byte scriptRowId) {
  // Script row is yet to be done by default unless specified otherwise onwards in this function.
  boolean isScriptRowDone = false;
  // Process the dataBytes array of the script row.
  byte duration = scriptRowArray[scriptRowId].dataBytes[0];
  boolean isInfinity = scriptRowArray[scriptRowId].dataBytes[1] & B00001000;
  // If the script row hasn't started yet
  if (scriptRowArray[scriptRowId].startTime == -1) {
    startScriptRow(scriptRowId);
    scriptRowArray[scriptRowId].startTime = getSYSTIM();
    if(isInfinity) {
      scriptRowArray[scriptRowId].duration = -1;
    } else {
      scriptRowArray[scriptRowId].duration = constrain(duration, MIN_TIME_VALUE, MAX_TIME_VALUE) * DURATION_MULTIPLIER;
    }
  } else {
    // if(scriptRowArray[scriptRowId].duration != -1 && scriptRowArray[scriptRowId].startTime + scriptRowArray[scriptRowId].duration < millis()) {
    if((scriptRowArray[scriptRowId].duration != -1) && (chk4TimeoutSYSTIM(scriptRowArray[scriptRowId].startTime,scriptRowArray[scriptRowId].duration) == SYSTIM_TIMEOUT)) {
      finishedScriptRow(scriptRowId);
      isScriptRowDone = true;
    }
  }
  return isScriptRowDone;
}

boolean processSound(byte scriptRowId) {
  // Script row is yet to be done by default unless specified otherwise onwards in this function.
  boolean isScriptRowDone = false;
  // Process the dataBytes array of the script row.
  byte trackNumber = scriptRowArray[scriptRowId].dataBytes[0];
  unsigned long soundRunTime = 0;
  // If the script row hasn't started yet
  if (scriptRowArray[scriptRowId].startTime == -1) {
    soundPlayer.setVolume(4);
    startScriptRow(scriptRowId);
    scriptRowArray[scriptRowId].startTime = getSYSTIM();
    soundPlayer.play(trackNumber, scriptRowId);
    debugRUN("Play sound\r\n");
  } else {
    // If the soundPlayer has finished playing the track
    soundRunTime = getSYSTIM()-scriptRowArray[scriptRowId].startTime;
    if(!soundPlayer.isPlaying() && soundPlayer.scriptRowId == scriptRowId) {
      if (soundRunTime<150)
      {
        delay(10000);
        soundRunTime = getSYSTIM()-scriptRowArray[scriptRowId].startTime;
        debugRUN("Sound play time is: ");
        debugRUN(soundRunTime);
        debugRUN("\r\n");
      }
      finishedScriptRow(scriptRowId);
      isScriptRowDone = true;
    }
    // If the soundPlayer is playing, but from another scriptRowId
    else if(soundPlayer.isPlaying() && soundPlayer.scriptRowId != scriptRowId) {
      numOfRunningScriptRows--;
      isScriptRowDone = true;
    }
  }
  return isScriptRowDone;
}

boolean processSequence(byte scriptRowId) {
  // Script row is yet to be done by default unless specified otherwise onwards in this function.
  boolean isScriptRowDone = false;
  // Process the dataBytes array of the script row.
  byte sequenceNumber = scriptRowArray[scriptRowId].dataBytes[0];
  // If the script row hasn't started yet
  if (scriptRowArray[scriptRowId].startTime == -1) {
    startScriptRow(scriptRowId);
    sequenceDone(scriptRowId); // Reset the sequence before we start it
    scriptRowArray[scriptRowId].startTime = getSYSTIM(); // Set the startTime of the 1st command in the sequence.
  } else {
    switch(sequenceNumber) {
      case 1:
        isScriptRowDone = garbageCanLifter(scriptRowId);
        break;
      case 2:
        isScriptRowDone = travelRandom(scriptRowId);        
        break;
      case 3:
        isScriptRowDone = basketball(scriptRowId);
        break;
      case 4:
        isScriptRowDone = goalKeeper(scriptRowId);
        break;
      case 5:
        isScriptRowDone = ballKicker(scriptRowId);
        break;
      case 6:
        isScriptRowDone = fan(scriptRowId);
        break;
      case 7:
        isScriptRowDone = squareMovement(scriptRowId);
        break;
      case 8:
        isScriptRowDone = ledMonkeyMove(scriptRowId);
        break;
      default:
        isScriptRowDone = true;
        break;
    }
    if(isScriptRowDone) {
      finishedScriptRow(scriptRowId);
    }
  }
  return isScriptRowDone;
}

void startScriptRow(byte scriptRowId) {
  numOfRunningScriptRows++;
  scriptRowArray[scriptRowId].isRunning = true;
  comHandler.sendRowExecute(OP_COMMAND_ROW_EXECUTION_START, scriptRowId);
}

void finishedScriptRow(byte scriptRowId) {
  numOfRunningScriptRows--;
  comHandler.sendRowExecute(OP_COMMAND_ROW_EXECUTION_END, scriptRowId);
}


boolean processMoveMotor(byte scriptRowId) 
{
    // Script row is yet to be done by default unless specified otherwise onwards in this function.
    boolean isScriptRowDone = false;
    // Process the dataBytes array of the script row.
    byte motorAndDirection[NUM_OF_MOTORS] = {255, 255, 255};
    byte direction = scriptRowArray[scriptRowId].dataBytes[0];
    byte power = scriptRowArray[scriptRowId].dataBytes[1];
    byte duration = scriptRowArray[scriptRowId].dataBytes[3];
    boolean isInfinity = scriptRowArray[scriptRowId].dataBytes[4] & B00001000;
    byte parsingMode = scriptRowArray[scriptRowId].dataBytes[7];
    int encA = 0;
    int encB = 0;
    int encC = 0;

    // If the script row hasn't started yet
    if (scriptRowArray[scriptRowId].startTime == -1) 
    {
        motorAndDirectionHelper(direction, motorAndDirection);
        startScriptRow(scriptRowId);
        // Check the parsing mode of the parameters
        //motorA_Power_flag = 0;
        switch(parsingMode) 
        {
            // case 0 - Simple time --> duration is a time for the motors, given from dataBytes[3].
            case 0: 
            {
                encoderDuration = 0;
                if(isInfinity) 
                {
                    scriptRowArray[scriptRowId].duration = -1;
                    isScriptRowDone = true;
                } 
                else 
                {
                    scriptRowArray[scriptRowId].duration = constrain(duration, MIN_TIME_VALUE, MAX_TIME_VALUE) * DURATION_MULTIPLIER;
                }
                break;
            }
            // case 1 - Random time --> duration is a random time between 1 - dataBytes[3] seconds \ 1 - 10 seconds by default if dataBytes[3] is 0
            case 1: 
            {
                if(duration) 
                {
                    duration = random(10, duration);
                } 
                else 
                {
                    duration = random(10, 101);
                }
                scriptRowArray[scriptRowId].duration = constrain(duration, MIN_TIME_VALUE, MAX_TIME_VALUE) * DURATION_MULTIPLIER;
                break;
            }
            // case 2 - Map Steps --> duration is the number of Steps on the Map the Algo robot will move.
            case 2: 
            {
                scriptRowArray[scriptRowId].duration = -1;
                encoderDuration = duration * mapStep;
                break;
            }
            // case 3 - Rotation Degrees --> duration is the number of degrees (divided by 10) the Algo robot will rotate.
            case 3: 
            {
                scriptRowArray[scriptRowId].duration = -1;
                encoderDuration = duration * tenDegrees;
                break;
            }
            // case 4 - Random Rotation Degrees --> duration is a random time between 1 - dataBytes[3] seconds \ 1 - 10 seconds by default if dataBytes[3] is 0
            case 4: 
            {
                if(duration) 
                {
                    duration = random(10, duration);
                } 
                else 
                {
                    duration = random(10, 101);
                }
                scriptRowArray[scriptRowId].duration = constrain(duration, MIN_TIME_VALUE, MAX_TIME_VALUE) * DURATION_MULTIPLIER;
                // Set direction random to Left or Right
                if(random(1, 100) % 2 == 0) 
                {
                    motorAndDirection[0] = 0;
                    motorAndDirection[1] = 1;
                } 
                else 
                {
                    motorAndDirection[0] = 1;
                    motorAndDirection[1] = 0;
                }
                break;
            }
            case (5):
            {
                Serial.print("Process move: ");
                Serial.println(getSYSTIM());
                scriptRowArray[scriptRowId].duration = -1;
                scriptRowArray[scriptRowId].encoder_cnt = scriptRowArray[scriptRowId].dataBytes[4];
                scriptRowArray[scriptRowId].encoder_cnt = (scriptRowArray[scriptRowId].encoder_cnt << 8) | scriptRowArray[scriptRowId].dataBytes[3];
                scriptRowArray[scriptRowId].encoder_cnt = scriptRowArray[scriptRowId].encoder_cnt * 2; // we have rising and falling edge
                break;
            }

        }  
        // After parsing all the dataBytes --> start the Motors
        //delay (100); // Before we run, we want a short delay, to make sure the previous motor finished and the current is stable. 

        scriptRowArray[scriptRowId].startTime = getSYSTIM(); // This is for knowing when to STOP the motors. 
        for(int i = 0; i < NUM_OF_MOTORS; i++) {
            if(motorAndDirection[i] != 255) {
                setMotorInterrupt(i, encoderDuration, scriptRowId);
                motors[i].start(motorPowerHelper(power), motorAndDirection[i], scriptRowId);
                debugRUN(F("Run motor ["));
                debugRUN(i);
                debugRUN(F("] with power ["));
                debugRUN(motorPowerHelper(power));
                debugRUN(F("]\r\n"));
            }
        }
        preDiff = 0; // So that next iteration also starts with PreDiff = 0;
        tempPowerA = motorPowerHelper(power); 
        tempPowerB = motorPowerHelper(power); 
        motorBalanceTimer = getSYSTIM(); // starting the timer for the motors. 
    } 
    else 
    {
        // If not infinity and the duration time has passed
        // if(scriptRowArray[scriptRowId].duration != -1 && scriptRowArray[scriptRowId].startTime + scriptRowArray[scriptRowId].duration < millis()) {
        if((scriptRowArray[scriptRowId].duration != -1) && (chk4TimeoutSYSTIM(scriptRowArray[scriptRowId].startTime,scriptRowArray[scriptRowId].duration) == SYSTIM_TIMEOUT)) 
        {
            motorAndDirectionHelper(direction, motorAndDirection);
            for(int i = 0; i < NUM_OF_MOTORS; i++) 
            {
                if(motorAndDirection[i] != 255) 
                {
                    motors[i].stop(scriptRowId);
                }
            }
            finishedScriptRow(scriptRowId);
            isScriptRowDone = true;
        }
    }
    encA = TCNT3; 
    encB = TCNT1;
    encC = TCNT4;
    diff = encA - encB;       
#ifdef TEST
    debugRUN(F("************************\r\n"));
    debugRUN(F("diff: ")); 
    debugRUN(diff); 
    debugRUN(F(" preDiff: ")); 
    debugRUN(preDiff);
    debugRUN(F(" ||  EncA: ")); 
    debugRUN(encA);  debugRUN(F("|| EncB: "));
    debugRUN(encB);  debugRUN(F("\r\n"));
#endif
    switch(parsingMode) 
    {
        case(3):
        case(4):
        {
            if ((chk4TimeoutSYSTIM(motorBalanceTimer,100) == SYSTIM_TIMEOUT) && (encoderDuration-encA>400) && (encoderDuration-encB>400)) 
            {      
                // balancing the motors
                // debugRUN(F("************************\r\n"));
                //changeSpeed = motorBalanceSpeed(stepPow, maxPower);
                changeSpeed = motorBalanceSpeed();
                if (changeSpeed==1) 
                {
                    debugRUN(F("New Powers. Pow_A: ")); 
                    debugRUN(tempPowerA); 
                    debugRUN(F("  ||  Pow_B: ")); 
                    debugRUN(tempPowerB); 
                    debugRUN(F("\r\n************************\r\n"));
                    motors[0].changeSpeed(tempPowerA); 
                    motors[1].changeSpeed(tempPowerB); 
                    motorBalanceTimer = getSYSTIM();      
                }  
            }    
            break;
        }
        case(5):
        {

            motorAndDirectionHelper(direction, motorAndDirection);
            if(motorAndDirection[0] != 255)  // motor A is active
            {
                if(encA >= scriptRowArray[scriptRowId].encoder_cnt)
                {
                    motors[0].stop(scriptRowId);
                    finishedScriptRow(scriptRowId);
                    isScriptRowDone = true;
                    Serial.print("Stop move: ");
                    Serial.println(getSYSTIM());
                }

            }
            if(motorAndDirection[1] != 255)  // motor B is active
            {
                if(encB >= scriptRowArray[scriptRowId].encoder_cnt)
                {
                    motors[1].stop(scriptRowId);
                    finishedScriptRow(scriptRowId);
                    isScriptRowDone = true;
                    Serial.print("Stop move: ");
                    Serial.println(getSYSTIM());
                }
            }
            if(motorAndDirection[2] != 255)  // motor C is active
            {
                if(encC >= scriptRowArray[scriptRowId].encoder_cnt)
                {
                    motors[2].stop(scriptRowId);
                    finishedScriptRow(scriptRowId);
                    isScriptRowDone = true;
                    Serial.print("Stop move: ");
                    Serial.println(getSYSTIM());
                }
            }
            break;
        }
    }
    return isScriptRowDone;
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


/* Private functions ******************************************************* */

/* ***************************** END OF FILE ******************************* */
