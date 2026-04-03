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

/* Exported functions ****************************************************** */
SoundPlayer::SoundPlayer(const byte txPin, const byte statePin) : soundSerial(txPin) {
    pinMode(txPin, OUTPUT);
    pinMode(statePin, INPUT);
    setVolume(4);
    soundSerial.begin(115200);
}

void SoundPlayer::sendTrackCommand(byte trackNumber) {
    char command[7] = {'p', ' ', '0', '0', '0', '0', '\0'};
    command[5] = '0' + (trackNumber % 10);
    trackNumber /= 10;
    command[4] = '0' + (trackNumber % 10);
    trackNumber /= 10;
    command[3] = '0' + (trackNumber % 10);
    trackNumber /= 10;
    command[2] = '0' + (trackNumber % 10);
    soundSerial.println(command);
}

void SoundPlayer::play(byte trackId, byte scriptRowId, boolean untillStop) {
    if(this->scriptRowId != 0 && isPlaying() && !untillStop) {
        prematureEndSoundScript(this->scriptRowId);
    }
    // Create the trackId that we got.
    if(trackId != 255) {
        if(untillStop) {
            if(!isPlaying()) {
                this->currentTrack = trackId;
                this->scriptRowId = scriptRowId;
                this->startMillis = getSYSTIM();
                this->isOn = true;
                sendTrackCommand(trackId);
            }
        } else {
            this->currentTrack = trackId;
            this->scriptRowId = scriptRowId;
            this->startMillis = getSYSTIM();
            this->isOn = true;
            sendTrackCommand(trackId);
        }
    }
    // If we didnt get a trackId, this play function will act as a stop to the currentTrack that is playing.
    else {
        stop();
    }
}
void SoundPlayer::stop() {
    this->scriptRowId = 0;
    this->startMillis = -1;
    if(isPlaying()) {
        sendTrackCommand(this->currentTrack); // If a track is playing, we send it again to stop it.
    }
    this->currentTrack = 0;
}
void SoundPlayer::setVolume(int volumeLevel) 
{
    debugSOUND(F("Set sound output value to: "));
    debugSOUND(volumeLevel);
    debugSOUND(F("\r\n"));
    switch (volumeLevel) {
        case 0: soundSerial.println('B'); break;
        case 1: soundSerial.println('D'); break;
        case 2: soundSerial.println('E'); break;
        case 3: soundSerial.println('I'); break;
        case 4: soundSerial.println('K'); break;
        case 5: soundSerial.println('L'); break;
        case 6: soundSerial.println('M'); break;
        case 7: soundSerial.println('P'); break;
        case 8: soundSerial.println('T'); break;
        default: soundSerial.println('t'); break;
    }
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
