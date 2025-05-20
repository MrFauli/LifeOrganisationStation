
#include "SoundPlayer.h"

SoundPlayer::SoundPlayer() :volume(i2s), decoder(&volume, new MP3DecoderHelix()),lvc(0.1f)
{
   volume.setVolumeControl(lvc);          // Lautstärkeregelung aktivieren
    volume.setVolume(0.4f);    

}

void SoundPlayer::stopSound(){

    if(playing){
    playing = false;

    Serial.println("Wiedergabe abgeschlossen");
    mp3File.close();}

}

void SoundPlayer::begin() {

    // SD Card initialisieren
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    SD_MMC.setPins(12, 11, 13);

    if (!SD_MMC.begin(root, true /* mode1bit */, false /* format_if_mount_failed */, SDMMC_FREQ_DEFAULT)) {
        Serial.println("Fehler beim Mounten der SD-Karte");
        while (1);
    }

}

void SoundPlayer::playMP3(const char* fileName) {
    begin();
    mp3File = SD_MMC.open(fileName);
    if (!mp3File) {
        Serial.println("Datei nicht gefunden");
        return;
    }
    Serial.println("Gefunden!");

    auto config = i2s.defaultConfig(TX_MODE);
    config.pin_ws = 2;
    config.pin_bck = 42;
    config.pin_mck = -1;
    config.pin_data =41;

    i2s.begin(config);
    decoder.begin();

    copier.begin(decoder, mp3File);
    playing = true;

  
}
void SoundPlayer::updatePlaying() {

    if(playing == true){
        if (!copier.copy()) {
            Serial.println("Wiedergabe abgeschlossen");
            stopSound();
        }
    }


}
