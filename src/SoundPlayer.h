#ifndef SOUNDPPLAYER_H
#define SOUNDPPLAYER_H

#include "AudioTools.h"
#include "AudioTools/AudioLibs/VS1053Stream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include <SD_MMC.h>
#include <Preferences.h>
#include "WiFi.h"

class SoundPlayer {
public:
    SoundPlayer();
    void stopSound();
    void begin();
    // void toogleRadio();
    void playMP3(const char* fileName);
    void updatePlaying();
private:
    I2SStream i2s;

    VolumeStream volume;
    LogarithmicVolumeControl lvc;
    EncodedAudioStream decoder;
    StreamCopy copier;
    StreamCopy stream; // Verbindung: URL -> Decoder -> i2s

    URLStream url;


    const char* root = "/root";
    bool playing = false;
    File mp3File;
    Preferences prefs;
};

#endif