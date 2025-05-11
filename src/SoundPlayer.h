#ifndef SOUNDPPLAYER_H
#define SOUNDPPLAYER_H

#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include <SD_MMC.h>



class SoundPlayer {
public:
    SoundPlayer();
    void stopSound();
    void begin();
    void playMP3(const char* fileName);
    void updatePlaying();
private:
    I2SStream i2s;

    VolumeStream volume;
    LogarithmicVolumeControl lvc;
    EncodedAudioStream decoder;
    StreamCopy copier;
    const char* root = "/root";
    bool playing = false;
    File mp3File;

};

#endif