/* Define to prevent recursive inclusion *********************************** */
#ifndef __THREAD_H
#define __THREAD_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include "pinout.h"

/* Exported constants ****************************************************** */
#define debugTHREAD                                    printDEBUG                      
/* #define debugTHREAD */                                                          

#define NUM_OF_THREADS 15


/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */
class Thread {
    public:
        boolean isRunning = false;
        byte scriptRowId = 0;
        byte threadBelongsToLoopRowId = 0;        // If a thread was opened within a loop, it belongs to it.
        byte loopRowIdThatOpenedInThisThread = 0; // Keeps the last loop that has been opened in this thread.
        uint8_t sequanceRunFlag;

        void start();
        void stop();
        void reset();
};

/* Exported variables ****************************************************** */
extern byte currentThread;
extern byte numOfRunningThreads;
extern Thread threadArray[NUM_OF_THREADS];
/* Exported functions ****************************************************** */
void threadsInit();
void assignScriptRow(byte threadId);
void goToEndLoop();
boolean isAllowedToContinue(byte startLoopScriptRowId, byte endLoopThreadId);

#endif 
/* ***************************** END OF FILE ******************************* */
