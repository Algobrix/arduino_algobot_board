/* Includes **************************************************************** */
#include "thread.h"
#include "scriptRow.h"
#include "system.h"

/* Private constants ******************************************************* */

/* Private macros ********************************************************** */

/* Private types *********************************************************** */

/* Private variables ******************************************************* */
byte currentThread = 0;
byte numOfRunningThreads = 0;
Thread threadArray[NUM_OF_THREADS];
/* Private function prototypes ********************************************* */

/* Exported functions ****************************************************** */
void Thread::start() {
    numOfRunningThreads++;
    this->isRunning = true;   // Set the thread as running.
    isPlaying = true;         // Brain is playing whenever a thread is running.
}

void Thread::stop() {
    numOfRunningThreads--;
    this->isRunning = false;
    if(numOfRunningThreads == 0 && numOfRunningScriptRows == 0) {
        stopPlaying();
    }
}

void Thread::reset() {
    // Reset the thread
    this->isRunning = false;
    this->scriptRowId = 0;
    this->threadBelongsToLoopRowId = 0;
    this->loopRowIdThatOpenedInThisThread = 0;
}


void threadsInit() {
  for (int i = 0; i < totalRowsInData; i++) {
    threadArray[i].reset();
  }
  currentThread = 0;
  numOfRunningThreads = 0;
}
void assignScriptRow(byte threadId) {
  // Iterate over the script row array to find the first script row that belongs to this thread.
  for (byte i = 0; i < totalRowsInData; i++) {
    if(scriptRowArray[i].type != 0 && scriptRowArray[i].threadId == threadId) {
      threadArray[threadId].scriptRowId = i;
      return;
    }
  }
  // If we didn't find the next script row for the current thread, hence the thread is done and needs to stop.
  threadArray[threadId].stop();
}
void goToEndLoop() {
  // Iterate over the script row array to find the end loop that matches the start loop
  for (int i = threadArray[currentThread].scriptRowId + 1; i < totalRowsInData; i++) {
    // If its TYPE_END_LOOP
    if(scriptRowArray[i].type == TYPE_END_LOOP) {
      // dataByte[0] of the end loop is the scriptRowId of his matching start loop
      if(scriptRowArray[i].dataBytes[0] == threadArray[currentThread].scriptRowId) {
        // dataByte[1] of the end loop is the threadId of his matching start loop
        if(scriptRowArray[i].dataBytes[1] == currentThread) {
          // FOUND THE END LOOP!
          // An end loop can be (not always) on a diffrent thread so we:
          byte endLoopThread = scriptRowArray[i].threadId;  // Save the thread of the end loop
          threadArray[currentThread].stop();                // Stop the currentThread
          threadArray[endLoopThread].scriptRowId = i;       // Set the scriptRowId that the thread of the endLoopThread runs, to the end loop.
          threadArray[endLoopThread].start();               // Start the thread of the end loop.
          break;
        }
      }
    }
  }
}
boolean isAllowedToContinue(byte startLoopScriptRowId, byte endLoopThreadId) {
  for(int i = 1; i < NUM_OF_THREADS; i++) {
    // If we find a thread that belongs to the given startLoopScriptRowId
    if(threadArray[i].threadBelongsToLoopRowId == startLoopScriptRowId) {
      // If the thread isRunning and is NOT the currentThread
      if(threadArray[i].isRunning && i != currentThread) {
        // Then we found a running thread --> not all threads are done!
        return false;
      }
    }
  }

  /* This is if we want to solve a case where an infinity block is inside a loop.
     // In this case, the infinity is a non-blocking command, thus the loop ignores it, during the iterations. 
     // If we want to change it to be a blocking command (blocking the loop next iteration) thus we need to do something as follows, it's not working yet. 
  for (int i = startLoopScriptRowId + 1; i < totalRowsInData; i++) {
    // If there is a scriptRowId running that is on a thread that belongs to the startLoopScriptRowId
    if(scriptRowArray[i].isRunning) {
      if(threadArray[scriptRowArray[i].threadId].threadBelongsToLoopRowId == startLoopScriptRowId) {
        if(i != currentThread) {
          return false;
        }
      }
    }
  }*/
  
  // The entire section checks if there is another loop within the loop, doesn't effect our current issue (20.01.2020)
  for(int i = startLoopScriptRowId + 1; i < totalRowsInData; i++) {
    // If we find another open loop after the loop that this end loop belongs to (a loop within loop)
    if(scriptRowArray[i].type == TYPE_START_LOOP) {
      // If the loop is in the same thread that this end loop thread belongs to
      if(scriptRowArray[i].threadId == threadArray[endLoopThreadId].threadBelongsToLoopRowId) {
        // We change the startTime of a start loop script to -1 when the end loop of it has been excuted and finished.
        // If the counter isn't 0 --> The loop within has yet to finish all its iterations --> can't continue.
        if(scriptRowArray[i].counter != 0) {
          return false;
        }

        // ############################################ IMPORTANT - README ############################################
        // If the loop within is NOT in the last iteration :
        // *  The thread that is waiting for the loop within to finish will start again in the next iteration of the loop within.
        // *  if we let that happen, we will have a thread running again before it stopped --> numOfRunningThreads will get 1 more false running thread!
        // *  When we stop() a thread, it can call stopPlaying() ONLY WHEN numOfRunningThreads is 0 and there is no running script row.
        // *  BUT, it will never happen when we added a false running thread to the numOfRunningThreads!
        // *  Therefore, we must remove this false running thread WITHOUT stopping the thread.
        // ############################################ IMPORTANT - README ############################################

        // Else --> the loop within has FINISHED
        else {
          // If this end loop (outter loop) is in the last iteration --> remove all the false running threads
          if(scriptRowArray[startLoopScriptRowId].duration == 0) {
            // The calculation below has been done by slowly debugging the loop within loop case...
            // In general: the number of false threads that will be opened are the iterations of the outer thread (which is startTime+1)
            // multiplied by the iterations of the inner thread minus 1 (which is startTime)!...
            numOfRunningThreads -= (scriptRowArray[i].startTime * (scriptRowArray[startLoopScriptRowId].startTime+1));
            debugTHREAD(F("numOfRunningThreads:  "));
            debugTHREAD(numOfRunningThreads);
            debugTHREAD(F("\r\n"));
          }
        }
      }
    }
  } 
  return true;
}


/* Private functions ******************************************************* */

/* ***************************** END OF FILE ******************************* */
