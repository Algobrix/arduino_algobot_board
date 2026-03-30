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
void SoundPlayer::play(byte trackId, byte scriptRowId, boolean untillStop) {
    if(this->scriptRowId != 0 && isPlaying() && !untillStop) {
        prematureEndSoundScript(this->scriptRowId);
    }
    // Create the trackId that we got.
    if(trackId != 255) {
        if(untillStop) {
            if(!isPlaying()) {
                String trackCommand = getTrackCommand(trackId);
                this->currentTrack = trackId;
                this->scriptRowId = scriptRowId;
                this->startMillis = getSYSTIM();
                this->isOn = true;
                this->soundSerial.println(trackCommand);
            }
        } else {
            String trackCommand = getTrackCommand(trackId);
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
    String trackCommand = getTrackCommand(this->currentTrack);
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
String SoundPlayer::getTrackCommand(byte trackNumber) {
    // The command that we get is:
    // p 00xx --> xx = number of the file
    String command = "p ";
    // Create starting 0's
    // Example: trackId is 10 (size of 2 chars\digits) the command will be "p 0010"
    for(int i = String(trackNumber).length(); i < 4; i++) {
        command += "0";
    }
    command += String(trackNumber);
    return command;
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
