/*
 * Company:
 *  Algobrix
 * 
 * Naming Conventions :
 * https://google.github.io/styleguide/cppguide.html
 * Following the above as much as possible
 */ 
#include "main.h"
#include "fwver.h"

void setup() 
{
    initDEBUG(115200);
    sleepTimeoutMillis = getSYSTIM();
    randomSeed(getSYSTIM());
    stopPlaying(true);
}

void loop() {
    algopython_fsm_update();
    comHandler.processPythonCommands();
    
}

