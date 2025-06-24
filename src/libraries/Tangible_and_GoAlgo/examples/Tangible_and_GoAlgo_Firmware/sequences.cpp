/* Includes **************************************************************** */
#include "sequences.h"
#include "systim.h"
#include "runTime.h"
#include "motor.h"
#include "comHandler.h"
#include "soundPlayer.h"
#include "sensor.h"
#include "led.h"
#include "thread.h"
#include "system.h"


/* Private constants ******************************************************* */


/* Private macros ********************************************************** */

/* Private types *********************************************************** */

/* Private variables ******************************************************* */
boolean isStartOfLoop = true;
byte loopCounter = 0;
int encA = 0; 
int encB = 0;
// parameter for garbage can
int playSound_11 =1; // for garbage can
// parameters for travel random
byte travelRandomCase = 0;
int yCurrent = 0; // X current
int xCurrent = 0; // Y current
int xRand = 0; 
int yRand = 0; 
int moveRand = 0; 
int sign = 0; 
int rand_dir = 0; // 0 - Up, 2 = 90 = Right, 3 = 180 = Down, 4 = -90 = 270 = Left
int current_dir = 0; // 0 - up, 2 = 90 = Right, 3 = 180 = Down, 4 = -90 = 270 = Left
//unsigned long blinkTimer ;


/* Private function prototypes ********************************************* */

/* Exported functions ****************************************************** */
// Helper Functions
void nextCommand(byte scriptRowId) {
  scriptRowArray[scriptRowId].duration = -1;
  scriptRowArray[scriptRowId].startTime = getSYSTIM();
  scriptRowArray[scriptRowId].counter += 1;
}
void sequenceDone(byte scriptRowId) {
  isStartOfLoop = true;
  travelRandomCase = 0;
  scriptRowArray[scriptRowId].duration = -1;
  scriptRowArray[scriptRowId].startTime = -1;
  scriptRowArray[scriptRowId].counter = 0;
}

void startLoopCommand(byte iterations) {
  if(isStartOfLoop) {
    loopCounter = iterations;
    isStartOfLoop = false;
  }
}
void endLoopCommand(byte scriptRowId, byte firstCommandInLoop) {
  if(loopCounter > 1) {
    scriptRowArray[scriptRowId].counter = firstCommandInLoop;
    loopCounter--;
  } else {
    isStartOfLoop = true;
    nextCommand(scriptRowId);
  }
}

// Single Commands Helper Functions
void moveMotorTime(byte scriptRowId, long duration, byte *motorAndDirection, byte pwm) {
  // If the command hasn't started yet
  if(scriptRowArray[scriptRowId].duration == -1) {
    comHandler.sendRowExecute(OP_COMMAND_ROW_EXECUTION_START, scriptRowId);
    scriptRowArray[scriptRowId].duration = duration;
    for(int i = 0; i < NUM_OF_MOTORS; i++) {
      if(motorAndDirection[i] != 255) {
        motors[i].start(pwm, motorAndDirection[i], scriptRowId);
      }
    }
  } else {
    // if(scriptRowArray[scriptRowId].startTime + scriptRowArray[scriptRowId].duration <= millis()) {
    if(chk4TimeoutSYSTIM(scriptRowArray[scriptRowId].startTime,scriptRowArray[scriptRowId].duration) == SYSTIM_TIMEOUT) {
     for(int i = 0; i < NUM_OF_MOTORS; i++) {
        if(motorAndDirection[i] != 255) {
          motors[i].stop(scriptRowId);
        }
      }
      nextCommand(scriptRowId); 
    }
  }
}

void ground_zero(boolean dir, byte scriptRowId){   //Lower to ground zero 
    // If the Command hasn't started yet.
  if(scriptRowArray[scriptRowId].duration == -1) 
  {
    comHandler.sendRowExecute(OP_COMMAND_ROW_EXECUTION_START, scriptRowId);
    scriptRowArray[scriptRowId].duration = -2;
    setMotorInterrupt(2, -1, scriptRowId);
    encoderDuration = 0;
	motorBalanceTimer = getSYSTIM();
	motors[2].start(150, dir, scriptRowId);
  } 
  else    {
	  // if(motorBalanceTimer + 50 <= millis())
    if(chk4TimeoutSYSTIM(motorBalanceTimer,50) == SYSTIM_TIMEOUT)
    {
      if(TCNT4 - encoderDuration <= 2) {
        setMotorInterrupt(2, 0, scriptRowId);
        motors[2].stop(scriptRowId);
        nextCommand(scriptRowId);
        } 
      else {
        encoderDuration = TCNT4;
        motorBalanceTimer = getSYSTIM();
        }
      }
    }
}

void moveMotorEncoder(byte scriptRowId, long encoderDuration, byte *motorAndDirection, byte pwm) {
  // If the command hasn't started yet
  boolean changeSpeed= 1; 
  if(scriptRowArray[scriptRowId].duration == -1) 
  {
    comHandler.sendRowExecute(OP_COMMAND_ROW_EXECUTION_START, scriptRowId);
	scriptRowArray[scriptRowId].duration = INFINITY;
    for(int i = 0; i < NUM_OF_MOTORS; i++) {
      if(motorAndDirection[i] != 255) {
        setMotorInterrupt(i, encoderDuration, scriptRowId);
        motors[i].start(pwm, motorAndDirection[i], scriptRowId);
        preDiff = 0; // So that next iteration also starts with PreDiff = 0;
        tempPowerA = pwm; 
        tempPowerB = pwm; 
        motorBalanceTimer = getSYSTIM(); // starting the timer for the motors. 
      }
    }
  } 
  else 
  {
	  Serial.print("Duration: ");
	  Serial.println(scriptRowArray[scriptRowId].duration);
    // if(scriptRowArray[scriptRowId].startTime + scriptRowArray[scriptRowId].duration <= millis()) {
    if(chk4TimeoutSYSTIM(scriptRowArray[scriptRowId].startTime,scriptRowArray[scriptRowId].duration) == SYSTIM_TIMEOUT) 
	{
      for(int i = 0; i < NUM_OF_MOTORS; i++) {
        if(motorAndDirection[i] != 255) {
          motors[i].stop(scriptRowId);
        }
      }
      Serial.println(F("nextCommand")); 
      nextCommand(scriptRowId);
      return; 
    }
  }
  encA = TCNT3; encB = TCNT1;
  diff = encA - encB;       

  // if ((motorBalanceTimer+100 <= millis()) & (encoderDuration-encA>400) & (encoderDuration-encB>400)) {      
  if ((chk4TimeoutSYSTIM(motorBalanceTimer,100) == SYSTIM_TIMEOUT) && (encoderDuration-encA>400) && (encoderDuration-encB>400)) {      
    // balancing the motors
    changeSpeed = motorBalanceSpeed();
    if (changeSpeed==1) {
      motors[0].changeSpeed(tempPowerA); 
      motors[1].changeSpeed(tempPowerB); 
      motorBalanceTimer = getSYSTIM();      
    }  
  }          
}

void ledOn(byte scriptRowId, long duration, boolean *isLedSelected, byte r, byte g, byte b) {
  // If the command hasn't started yet
  if(scriptRowArray[scriptRowId].duration == -1) {
    comHandler.sendRowExecute(OP_COMMAND_ROW_EXECUTION_START, scriptRowId);
    scriptRowArray[scriptRowId].duration = duration;
    for(int i = 0; i < NUM_OF_LEDS; i++) {
      if(isLedSelected[i]) {
        leds[i].start(r, g, b, scriptRowId);
      }
    }
  } else {
    // if(scriptRowArray[scriptRowId].startTime + scriptRowArray[scriptRowId].duration <= millis()) {
    if(chk4TimeoutSYSTIM(scriptRowArray[scriptRowId].startTime,scriptRowArray[scriptRowId].duration) == SYSTIM_TIMEOUT ) {
      for(int i = 0; i < NUM_OF_LEDS; i++) {
        if(isLedSelected[i]) {
          leds[i].stop(scriptRowId);
        }
      }
      nextCommand(scriptRowId);
    }
  }
}

// Sequences Functions

boolean garbageCanLifter(byte scriptRowId) {
  /**
   * Cases:  
   * 0. Squeeze the garbage can to the ground
   * 1. Rotate Right 180 degrees
   * 2. Play Sound - Lift (Forever) & Raise the lift 45 degrees above the robot
   * 3. Stop the soundPlayer
   * 4-5. Play Sound - Shake (Forever) & Shake the can backwards
   * 6. Stop the soundPlayer
   * 7-8. Lower the lift to ground zero 
   * 9. Play Sound - Truck Reverse (Forever) & Rotate Left 180 degrees
   * 10. stop soundPlayer
   * 11. lift to 45 deg
   */
  boolean isSequenceDone = false;
//boolean isLedSelected[NUM_OF_LEDS] = {false, false};
  byte motorAndDirection[NUM_OF_MOTORS] = {255, 255, 255};
    
  switch(scriptRowArray[scriptRowId].counter) {
     case 0: { //Lower the lift to ground zero 
      if(scriptRowArray[scriptRowId].duration == -1) { // this allows working in multi thread to another action
        leds[0].start(255, 0, 0, scriptRowId);
        leds[1].start(255, 0, 0, scriptRowId);
      }
      ground_zero(CW,scriptRowId);
      break; 
    }
    case 1: { 
      // lift the garbage a bit from the ground. 
      motorAndDirection[2] = CCW; // CCW - UP
      moveMotorEncoder(scriptRowId, 6200, motorAndDirection, 170);
      break;
    }
    case 2: { 
      //  Rotate Right 180 degrees
      //soundPlayer.setVolume(4);
      soundPlayer.play(12, scriptRowId, true);
      motorAndDirection[0] = CW;
      motorAndDirection[1] = CCW;
      moveMotorEncoder(scriptRowId, 18 * tenDegrees+220, motorAndDirection, 210);
      //debugSEQUENCESln(F("Case Rotate "));
      
      break;
    }
    case 3: {
      // stop sound
      soundPlayer.stop();
      nextCommand(scriptRowId);
      break;
    }
    case 4: {      // Play Sound - Lift (Forever) & Raise the lift 45 degrees above the robot
      //motorAndDirection[2] = CCW; // CCW - UP
      //moveMotorEncoder(scriptRowId, 3300, motorAndDirection, 210);      
      //debugSEQUENCESln(F("Case 4 lift + sound"));  
      //debugSEQUENCES(F("playSound_11:  "));  debugSEQUENCESln(playSound_11);  
      if (playSound_11 < 500) {
        soundPlayer.play(11, scriptRowId, true);
        playSound_11 = playSound_11+1; 
      }
      ground_zero(CCW,scriptRowId); // CCW - UP
      break;
    }
    case 5: {
      // Stop the soundPlayer
      soundPlayer.stop();
      playSound_11 = 0;
      nextCommand(scriptRowId);
      break;
    }
    case 6: {
      // Play Sound - Shake (Forward) & Shake the can backwards
      soundPlayer.play(13, scriptRowId, true);
      //debugSEQUENCESln(F("Case 5, shake that thing"));
      nextCommand(scriptRowId);
      motorAndDirection[2] = CW;
      startLoopCommand(5);
      
      if(loopCounter % 2 == 1) {
        motorAndDirection[2] = CCW;
      }
      moveMotorEncoder(scriptRowId, 5, motorAndDirection, 200);
      delay (200);
      break;
    }
    case 7: { 
      //debugSEQUENCESln(F("Case 6, stop"));
      delay (200);
      endLoopCommand(scriptRowId, 6);
      break;
    }
    case 8: { // Stop the soundPlayer
      soundPlayer.stop();
      nextCommand(scriptRowId);
      break;
    }
    case 9: { // Lower the lift to ground zero    
      motorAndDirection[2] = CW; // CCW - UP
      //moveMotorEncoder(scriptRowId, 4200, motorAndDirection, 150);
      moveMotorEncoder(scriptRowId, 5200, motorAndDirection, 150);
      //debugSEQUENCESln(F("Case 8, Lower the lift 1500"));
      break; 
    } 
    /*case 10: {
      //nextCommand(scriptRowId);
      //motorAndDirection[2] = CCW; // CCW - UP
      //moveMotorEncoder(scriptRowId, 1000, motorAndDirection, 200);
      //ground_zero(CW,scriptRowId);
      break; 
    }*/

    case 10: { // Rotate left 180
      soundPlayer.play(12, scriptRowId, true);
      motorAndDirection[0] = CCW;
      motorAndDirection[1] = CW;
      moveMotorEncoder(scriptRowId, 19 * tenDegrees+100 , motorAndDirection, 200);
      break;
    }
    case 11: {
      soundPlayer.stop();
      nextCommand(scriptRowId);
      break;
    }
    case 12: {
      ground_zero(CW,scriptRowId);
      break; 
    } 
    case 13: {
      motorAndDirection[2] = CCW; // CCW - UP
      moveMotorEncoder(scriptRowId, 2200, motorAndDirection, 170);
      break; 
    }
    default: {
      leds[0].stop(scriptRowId);
      leds[1].stop(scriptRowId);
      sequenceDone(scriptRowId);
      isSequenceDone = true;
      break;
    }
  } 
  return isSequenceDone;
}

boolean travelRandom(byte scriptRowId) {
  boolean isSequenceDone = false;
//boolean isLedSelected[NUM_OF_LEDS] = {false, false};
  byte motorAndDirection[NUM_OF_MOTORS] = {255, 255, 255};
  
  switch(scriptRowArray[scriptRowId].counter) {
    case 0: { // move (forward / backward)
      startLoopCommand(5);  
      if((scriptRowArray[scriptRowId].duration == -1) && (loopCounter > 0)) { // this will activate the rand only once
        moveRand = random(1, 6); // 6 Different cases M1, M2, M3, -M1, -M2, -M3   
        if (moveRand>3){ // Move M1, M2, M3
          moveRand = moveRand - 3;  // moveRand-3 since: M3 = 6-3 -> 3 steps, M1 = 4-3 -> 1 step
          motorAndDirection[0] = CCW; // forward, current_dir =0 ; 
          motorAndDirection[1] = CCW;
        }
        else        { // Move -M1, -M2, -M3
          moveRand = moveRand - 4; // moveRand-4 since: M3 = 3-4 -> - 1 step, M1 = 1-4 -> -3 steps
          motorAndDirection[0] = CW; // forward, current_dir =0 ; 
          motorAndDirection[1] = CW;
        }
        if (current_dir == 0){ // face forward
          if ( (yCurrent + moveRand <=2) && (yCurrent + moveRand >=-2) )      {
             yCurrent = yCurrent + moveRand;
           } else {moveRand = 0;
           scriptRowArray[scriptRowId].counter = 0;}
       }
        if ((current_dir == 180)|| (current_dir == -180)){
          if ( (yCurrent - moveRand <=2) && (yCurrent - moveRand >=-2) )      {
            yCurrent = yCurrent - moveRand;
            } else {moveRand = 0;
            scriptRowArray[scriptRowId].counter = 0;}
        }
        if ((current_dir == 90)|| (current_dir == -270)){
          if ( (xCurrent + moveRand <=2) && (xCurrent + moveRand >=-2) )      {
             xCurrent = xCurrent + moveRand;
          } else {moveRand = 0;}
        }      
        if ((current_dir == -90)|| (current_dir == 270)){ 
          if ( (xCurrent - moveRand <=2) && (xCurrent - moveRand >=-2) )      {
            xCurrent = xCurrent - moveRand;
            } else {moveRand = 0;}
          }
      }
      if (abs(moveRand) > 0) {
        moveMotorEncoder(scriptRowId, abs(moveRand) * mapStep, motorAndDirection, 200);
        }
      else {
        debugSEQUENCES(F("Move was 0, we try again\r\n"));
      }
      break;
    } // close case 0
    case 1:
    {
    if ( (xCurrent + yCurrent) % 2 == 0 ) 
    { 
      //soundPlayer.play(14, scriptRowId, true);
      delay (2000); 
    }
    nextCommand(scriptRowId); 
    soundPlayer.stop();
    break;
    }
    case 2: { // rotate  random    
      if((scriptRowArray[scriptRowId].duration == -1) && (loopCounter > 0)) { // this will activate the rand only once
        soundPlayer.stop();
        rand_dir = random(1, 6); // -360, -180, -90, 90, 180, 360
        if (rand_dir > 3) {
          motorAndDirection[0] = CCW;
          motorAndDirection[1] = CW;
          rand_dir = (rand_dir - 3)*9; // creating 90, 180, 270
        }
        else {
          motorAndDirection[0] = CW;
          motorAndDirection[1] = CCW;
          rand_dir = (rand_dir - 4)*9; // creating -90, -180, -270
        }    
        current_dir = current_dir + rand_dir*10; 
        if (current_dir>360)
        {current_dir = current_dir -360; }
        else {
          if (current_dir<-360) {
            current_dir = current_dir +360;
          }
        }   
      }
      moveMotorEncoder(scriptRowId,  abs(rand_dir*tenDegrees), motorAndDirection, 210);
      break;
    } // close case 1
    case 3: { // do the loop
      endLoopCommand(scriptRowId, 0); // endLoopCommand(scriptRowId, case number to go to)  
      break;
    }
    case 4: { // Return Home - Rotate on Y Axis
      delay (1000); 
      if((scriptRowArray[scriptRowId].duration == -1) && (loopCounter > 0)) { // this will activate the following only once
        leds[0].start(255, 0, 0, scriptRowId);
        leds[1].start(255, 0, 0, scriptRowId);
        if (current_dir < 0)
        { current_dir = current_dir +360; } // making sure possible angles are 90, 180 or 270
        
        rand_dir = 180 - current_dir;
        motorAndDirection[0] = CCW; // 
        motorAndDirection[1] = CW;
        if (yCurrent < 0) {
            rand_dir = rand_dir + 180;
        }
        if (rand_dir == 360) {             rand_dir = 0;           }
        current_dir = current_dir + rand_dir;
        if (current_dir == 360) {             current_dir = 0;           }
        if ((rand_dir == 270) || (rand_dir == -90))
        { 
          rand_dir = 90; // rotating -90, thus changing the motors
          motorAndDirection[0] = CW;  
          motorAndDirection[1] = CCW;
        }     
      }
      if (rand_dir >0) {
        moveMotorEncoder(scriptRowId,  abs(rand_dir/10)*tenDegrees, motorAndDirection, 210);
      }
      else {            nextCommand(scriptRowId); }
      break;
    }
    case 5: { // Move on Y axis to the home postion     
      if((scriptRowArray[scriptRowId].duration == -1) && (loopCounter > 0)) { // this will activate the following only once
        moveRand = abs(0-yCurrent);
        motorAndDirection[0] = CCW; 
        motorAndDirection[1] = CCW;
//        debugSEQUENCES(F("Move Y at : ")); debugSEQUENCESln(moveRand);
      }      
      if (moveRand>0) {
        moveMotorEncoder(scriptRowId,  moveRand* mapStep, motorAndDirection, 210);
      }
      else {            nextCommand(scriptRowId); }
      
      break;
    }
    case 6: { // Return Home - Rotate on X Axis      
      if((scriptRowArray[scriptRowId].duration == -1) && (loopCounter > 0)) { // this will activate the following only once
        rand_dir = 9; // rotate 90 deg
        if ( ((current_dir == 180) && (xCurrent<0)) || ((current_dir == 0) && (xCurrent>0)) ) { 
              // rotating -90, thus changing the motors
              motorAndDirection[0] = CW;  
              motorAndDirection[1] = CCW;
  //            debugSEQUENCES(F("rotating X at : ")); debugSEQUENCESln(-rand_dir*10);      
            }
        else {   // rotating 90, thus changing the motors
              motorAndDirection[0] = CCW;  
              motorAndDirection[1] = CW;
    //          debugSEQUENCES(F("rotating X at : ")); debugSEQUENCESln(rand_dir*10);      
        }
        
      }
      if (rand_dir >0) {
        moveMotorEncoder(scriptRowId,  abs(rand_dir*tenDegrees), motorAndDirection, 210);
      }
      else {            nextCommand(scriptRowId); }
      break;
    }
    case 7: { // Move on X axis to the home postion     
      if((scriptRowArray[scriptRowId].duration == -1) && (loopCounter > 0)) { // this will activate the following only once
        moveRand = abs(0-xCurrent);
        motorAndDirection[0] = CCW;  
        motorAndDirection[1] = CCW;
      //  debugSEQUENCES(F("Move X at : ")); debugSEQUENCESln(moveRand);
      }
      if (moveRand>0) {
        moveMotorEncoder(scriptRowId,  moveRand* mapStep, motorAndDirection, 210);
      }
      else {            nextCommand(scriptRowId); }
      break;
    }
    case 8: { // clean parameters for next round
      leds[0].stop(scriptRowId);
      leds[1].stop(scriptRowId);
      current_dir = 0; 
      xCurrent = 0; 
      yCurrent = 0; 
      moveRand = 0; 
      nextCommand(scriptRowId); 
      break;
    }    
    default: {
      sequenceDone(scriptRowId);
      isSequenceDone = true;
      break;
    }// close default 
  } // close switch
  return isSequenceDone;
}

boolean basketball(byte scriptRowId) {
  /**  
   * THREAD 1: 
   *          1) Clap 4 times - Motor C CW
   *          2) Play Sound
   * THREAD 2:
   *          1) Blink Red -> Blue -> Green
   * Cases:
   * 0. Play Sound - Clapping (Forever) & Start Motor C with Encoder (For Claps)
   * 1. Loop that Blinks (R->G->B) while Clapping.
   * 2. Stop soundPlayer
   */
  boolean isSequenceDone = false;
  switch(scriptRowArray[scriptRowId].counter) {
    case 0: {
      soundPlayer.play(15, scriptRowId);
      setMotorInterrupt(2, 7000, scriptRowId);
      motors[2].start(255, CCW, scriptRowId);
      nextCommand(scriptRowId);
      break;
    }
    case 1: {
      soundPlayer.play(15, scriptRowId, true);
      if(scriptRowArray[scriptRowId].duration == -1) {
        comHandler.sendRowExecute(OP_COMMAND_ROW_EXECUTION_START, scriptRowId);
        startLoopCommand(255); // loopCounter is initialized to 255
        scriptRowArray[scriptRowId].duration = 250; // this is the gap between each blink, 250 mili      
        if(loopCounter % 3 == 0) {
          leds[0].start(255, 0, 0, scriptRowId);
          leds[1].start(255, 0, 0, scriptRowId);
        } else if (loopCounter % 3 == 1) {
          leds[0].start(0, 255, 0, scriptRowId);
          leds[1].start(0, 255, 0, scriptRowId);
        } else if (loopCounter % 3 == 2) {
          leds[0].start(0, 0, 255, scriptRowId);
          leds[1].start(0, 0, 255, scriptRowId);
        }
      } else {
        // as long as the motors are running reduce loopcounter by 1.
        // if(scriptRowArray[scriptRowId].startTime + scriptRowArray[scriptRowId].duration <= millis()) {
        if(chk4TimeoutSYSTIM(scriptRowArray[scriptRowId].startTime,scriptRowArray[scriptRowId].duration) == SYSTIM_TIMEOUT) {
          if(motors[2].isRunning()) {
            nextCommand(scriptRowId); // initialis .duration to -1 and startTime to millis and counter+1
            leds[0].stop(scriptRowId);
            leds[1].stop(scriptRowId);
            loopCounter--;
            scriptRowArray[scriptRowId].counter = 1; // keeps us in case number 1
          } else {
            leds[0].stop(scriptRowId);
            leds[1].stop(scriptRowId);
            motors[2].stop(scriptRowId);
            nextCommand(scriptRowId);
          }
        }
      }
      break;
    }
    case 2: {
      soundPlayer.stop();
      nextCommand(scriptRowId);
      break;
    }
    default: {
      sequenceDone(scriptRowId);
      isSequenceDone = true;
      break;
    }
  }
  return isSequenceDone;
}

boolean goalKeeper(byte scriptRowId) {
  /**
   * 0-1.   Ground zero + start clapping sound
   * 2-3. Play Sound - Clapping (Forever) & Raise flag up (half spin of the motor)
      
   */
  boolean isSequenceDone = false;  
  isSequenceDone = basketball(scriptRowId);
  return isSequenceDone;
}

boolean ballKicker(byte scriptRowId) {
  /**
   * 0. Return the kicker to ground zero.
   * 1. Play sound - Kick (Forever) & Kick the ball & Blink LED's
   * 2. Return the kicker backwards.
   */ 
  boolean isSequenceDone = false;
  byte motorAndDirection[NUM_OF_MOTORS] = {255, 255, 255};
  switch(scriptRowArray[scriptRowId].counter) {
    case 0: { // ground zero
      motorAndDirection[2] = CW;
      moveMotorTime(scriptRowId, 500, motorAndDirection, 85);
      break;
    }
    case 1: { // kick + Sound + led
      motorAndDirection[2] = CCW;
      if(scriptRowArray[scriptRowId].duration == -1) { // this allows working in multi thread to another action
        soundPlayer.play(16, scriptRowId, true);
        leds[0].start(0, 255, 0, scriptRowId);
        leds[1].start(0, 255, 0, scriptRowId);
      }
      moveMotorTime(scriptRowId, 400, motorAndDirection, 255);
      break;
    }
    case 2: { // back to zero
      leds[0].stop(scriptRowId);
      leds[1].stop(scriptRowId);
      motorAndDirection[2] = CW;
      moveMotorTime(scriptRowId, 1000, motorAndDirection, 85);
      //moveMotorEncoder(scriptRowId, 500, motorAndDirection, 85);
      break;
    }
    default: {
      sequenceDone(scriptRowId);
      isSequenceDone = true;
      break;
    }
  }
  return isSequenceDone;
}

boolean fan(byte scriptRowId) {
  /**
   * Move Motor C CW 2 Seconds
   * 0. Move Motor C 2 Seconds
   */ 
  boolean isSequenceDone = false;
  byte motorAndDirection[NUM_OF_MOTORS] = {255, 255, 255};
  switch(scriptRowArray[scriptRowId].counter) {
    case 0: {
      motorAndDirection[2] = CW;
      moveMotorTime(scriptRowId, 2000, motorAndDirection, 255);
      break;
    }
    default: {
      sequenceDone(scriptRowId);
      isSequenceDone = true;
      break;
    }
  }
  return isSequenceDone;
}

boolean squareMovement(byte scriptRowId) {
  /**
   * Loop 4 { Move forward 2 Blocks, Rotate 90 Degrees } & Play sound - Happy Music (Forever)
   * 0. Start Loop (4) & Move Forward 2 Steps
   * 1. Rotate 90 Degrees Right (CW)
   * 2. End Loop Command
   */ 
  boolean isSequenceDone = false;
  byte motorAndDirection[NUM_OF_MOTORS] = {255, 255, 255};
  Serial.print("Script ID: ");
  Serial.println(scriptRowArray[scriptRowId].counter);
  Serial.print("Timer: ");
  Serial.println(scriptRowArray[scriptRowId].startTime);
  Serial.print("MIllis: ");
  Serial.println(millis());
  switch(scriptRowArray[scriptRowId].counter) {
    case 0: {
      soundPlayer.play(14, scriptRowId, true);
      motorAndDirection[0] = CCW;
      motorAndDirection[1] = CCW;
      startLoopCommand(4);
      moveMotorEncoder(scriptRowId, 2 * mapStep, motorAndDirection, 200);
      break;
    }
    case 1: {
      soundPlayer.play(14, scriptRowId, true);
      motorAndDirection[0] = CW;
      motorAndDirection[1] = CCW;
      moveMotorEncoder(scriptRowId, 9 * tenDegrees, motorAndDirection, 170);
      break;
    }
    case 2: {
      endLoopCommand(scriptRowId, 0);
      break;
    }
    default: {
      soundPlayer.stop();
      sequenceDone(scriptRowId);
      isSequenceDone = true;
      break;
    }
  }
  return isSequenceDone;
}


boolean ledMonkeyMove(byte scriptRowId) {
  /**
   * Infinity Loop { LED Red 2 Sec, Sound of Monkey, THREAD 1: Move Forward 3 -- THREAD 2: Wait Sensor 1 LOW, Rotate 90 Left }
   * 0. LED Red 2 Seconds
   * 1. Sound of monkey (Blocking)
   * 2. Move Forward 3 & Wait Sensor LOW
   * 3. Rotate 90 Left
   * 4. End Loop (Jump to command 0)
   */
  boolean isSequenceDone = false;
  byte motorAndDirection[NUM_OF_MOTORS] = {255, 255, 255};
  boolean isLedSelected[NUM_OF_LEDS] = {false, false};
  switch(scriptRowArray[scriptRowId].counter) {
    case 0: {
      isLedSelected[0] = true;
      isLedSelected[1] = true;
      ledOn(scriptRowId, 2000, isLedSelected, 255, 0, 0);
      break;
    }
    case 1: {
      if(scriptRowArray[scriptRowId].duration == -1) {
        comHandler.sendRowExecute(OP_COMMAND_ROW_EXECUTION_START, scriptRowId);
        scriptRowArray[scriptRowId].duration = -2;
        
      } else {
        if(!soundPlayer.isPlaying()) {
          nextCommand(scriptRowId);
        }
      }
      break;
    }
    case 2: {
      if(scriptRowArray[scriptRowId].duration == -1) {
        comHandler.sendRowExecute(OP_COMMAND_ROW_EXECUTION_START, scriptRowId);
        scriptRowArray[scriptRowId].duration = -2;
        setMotorInterrupt(0, 3 * mapStep, scriptRowId);
        setMotorInterrupt(1, 3 * mapStep, scriptRowId);
        motors[0].start(255, CCW, scriptRowId);
        motors[1].start(255, CW, scriptRowId);
      } else {
        uint8_t sensor_type = 0;
        byte sensorVal = sensors[0].getValue(&sensor_type);
        // If Sensor 1 Matches LOW
        if( 0 < sensorVal && sensorVal < 3) {
          nextCommand(scriptRowId);
        }
        // else if(scriptRowArray[scriptRowId].startTime + scriptRowArray[scriptRowId].duration <= millis()) {
        else if(chk4TimeoutSYSTIM(scriptRowArray[scriptRowId].startTime,scriptRowArray[scriptRowId].duration) == SYSTIM_TIMEOUT) {
          motors[0].stop(scriptRowId);
          motors[1].stop(scriptRowId);
          nextCommand(scriptRowId);
//        scriptRowArray[scriptRowId];
        }
      }
      break;
    }
    case 3: {
      motorAndDirection[0] = CCW;
      motorAndDirection[1] = CCW;
      moveMotorEncoder(scriptRowId, 9 * tenDegrees, motorAndDirection, 170);
      break;
    }
    case 4: {
      nextCommand(scriptRowId);
      //scriptRowArray[scriptRowId].counter = 0;
      break;
    }
    default: {
      sequenceDone(scriptRowId);
      isSequenceDone = true;
      break;
    }
  }
  return isSequenceDone;
}

/* Private functions ******************************************************* */

/* ***************************** END OF FILE ******************************* */
