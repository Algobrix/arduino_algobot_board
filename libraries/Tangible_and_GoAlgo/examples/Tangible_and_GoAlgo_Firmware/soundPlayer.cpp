/* Includes **************************************************************** */
#include "soundPlayer.h"
#include "systim.h"
#include "comHandler.h"

/* Private constants ******************************************************* */


/* Private macros ********************************************************** */

/* Private types *********************************************************** */

/* Private variables ******************************************************* */
SoundPlayer soundPlayer(SOUND_TX_PIN, SOUND_STATE_PIN);
/* Private function prototypes ********************************************* */

namespace {
/* "p " + 4 decimal digits + NUL; no heap */
void fillTrackCommand(char buf[8], byte trackNumber) {
    buf[0] = 'p';
    buf[1] = ' ';
    buf[2] = char('0' + (trackNumber / 1000) % 10);
    buf[3] = char('0' + (trackNumber / 100) % 10);
    buf[4] = char('0' + (trackNumber / 10) % 10);
    buf[5] = char('0' + (trackNumber % 10));
    buf[6] = '\0';
}
}  // namespace

/* Exported functions ****************************************************** */
SoundPlayer::SoundPlayer(const byte txPin, const byte statePin) : soundSerial(txPin) {
    pinMode(txPin, OUTPUT);
    pinMode(statePin, INPUT);
    setVolume(4);
    soundSerial.begin(115200);
}
void SoundPlayer::play(byte trackId, byte scriptRowId, boolean untillStop) {
    if(this->scriptRowId != 0 && isPlaying() && !untillStop) {
        prematureEndSoundScript(this->scriptRowId);
    }
    // Create the trackId that we got.
    if(trackId != 255) {
        char trackCommand[8];
        if(untillStop) {
            if(!isPlaying()) {
                fillTrackCommand(trackCommand, trackId);
                this->currentTrack = trackId;
                this->scriptRowId = scriptRowId;
                this->startMillis = getSYSTIM();
                this->isOn = true;
                this->soundSerial.println(trackCommand);
            }
        } else {
            fillTrackCommand(trackCommand, trackId);
            this->currentTrack = trackId;
            this->scriptRowId = scriptRowId;
            this->startMillis = getSYSTIM();
            this->isOn = true;
            this->soundSerial.println(trackCommand);
        }
    }
    // If we didnt get a trackId, this play function will act as a stop to the currentTrack that is playing.
    else {
        stop();
    }
}
void SoundPlayer::stop() {
    char trackCommand[8];
    fillTrackCommand(trackCommand, this->currentTrack);
    this->scriptRowId = 0;
    this->startMillis = -1;
    if(isPlaying()) {
        this->soundSerial.println(trackCommand); // If a track is playing, we send it again to stop it.
    }
    this->currentTrack = 0;
}
void SoundPlayer::setVolume(int volumeLevel) 
{
    debugSOUND(F("Set sound output value to: "));
    debugSOUND(volumeLevel);
    debugSOUND(F("\r\n"));
    soundSerial.println(volume[volumeLevel]);
}
boolean SoundPlayer::isPlaying() {
    // Short delay to let the state pin go high
    // if (startMillis + 100 < getSYSTIM()) {
    if (chk4TimeoutSYSTIM(startMillis,100) == SYSTIM_TIMEOUT) {
        isOn  = (digitalRead(SOUND_STATE_PIN) == LOW); // Pin is LOW when playing.
    }
    return isOn;
}

void prematureEndSoundScript(byte scriptRowId) {
  soundPlayer.stop();
  comHandler.sendRowExecute(OP_COMMAND_ROW_EXECUTION_PREMATURE_END, scriptRowId);
}

/* Private functions ******************************************************* */

/* ***************************** END OF FILE ******************************* */
