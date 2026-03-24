/* Define to prevent recursive inclusion *********************************** */
#ifndef __SOUNDPLAYER_H
#define __SOUNDPLAYER_H

/* Includes **************************************************************** */
#include <Arduino.h>
#include "pinout.h"
#include <SendOnlySoftwareSerial.h>

/* Exported constants ****************************************************** */
#define debugSOUND                                    printDEBUG                      

/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */
class SoundPlayer {
    private:
        SendOnlySoftwareSerial soundSerial;
        unsigned long startMillis = -1;
        boolean isOn = false;
        byte currentTrack = 0;
        char volume[10] = {'B', 'D', 'E', 'I', 'K', 'L', 'M', 'P', 'T', 't'}; // 'B'(LOWEST)...'t'(HIGHEST-DEFAULT)

    public:
        byte scriptRowId = 0;
        SoundPlayer(const byte txPin, const byte statePin);
        void play(byte trackId, byte scriptRowId, boolean untillStop=false);
        void stop();
        void setVolume(int volumeLevel);
        String getTrackCommand(byte trackNumber);
        boolean isPlaying();
};


/* Exported variables ****************************************************** */
extern SoundPlayer soundPlayer;

/* Exported functions ****************************************************** */
void prematureEndSoundScript(byte scriptRowId);

#endif 
/* ***************************** END OF FILE ******************************* */
