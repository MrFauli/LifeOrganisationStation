/**
 * @file player-sd-i2s.ino
 * @brief example using the SD library
 * 
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

#include "AudioTools.h"
#include "AudioTools/Disk/AudioSourceSD.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include <SD_MMC.h>
#include <PINS_JC4827W543.h>  
const char *root = "/root"; // Do not change this, it is needed to access files properly on the SD card
const char *AVI_FOLDER = "/avi";
size_t output_buf_size;
uint16_t *output_buf;

#define MAX_FILES 10 // Adjust as needed

String aviFileList[MAX_FILES];
int fileCount = 0;
int selectedIndex = 0;

I2SStream i2s;
MP3DecoderHelix decoder;

void printMetaData(MetaDataType type, const char* str, int len){
  Serial.print("==> ");
  Serial.print(toStr(type));
  Serial.print(": ");
  Serial.println(str);
}

void setup() {
  Serial.begin(115200);
  
  while (!Serial) {
    delay(10);  // Warte, bis USB verbunden ist
  }
  inMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SD_MMC.setPins(SD_SCK, SD_MOSI /* CMD */, SD_MISO /* D0 */);
  if (!SD_MMC.begin(root, true /* mode1bit */, false /* format_if_mount_failed */, SDMMC_FREQ_DEFAULT))
  {
    Serial.println("ERROR: SD Card mount failed!");
    while (true)
    {

    }
  }
  else{Serial.println("ja...");
        }
  File file = SD_MMC.open("/Monte-Wecker.mp3");
  if (!file) {
    Serial.println("Datei konnte nicht geöffnet werden!");
    while (true);
  }
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
  auto cfg = out.defaultConfig();
  cfg.sample_rate = 44100;
  cfg.bits_per_sample = 16;
  cfg.channels = 2;
  out.begin(cfg);

  // Decoder starten
  decoder.begin();

  // Datei abspielen
  copier.begin();
  copier.copy(file);
  copier.end();

  file.close();
}

void loop() {
  player.copy();
}

void setup() {
// SD Card initialization
  Serial.begin(115200);
  
  while (!Serial) {
    delay(10);  // Warte, bis USB verbunden ist
  }
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SD_MMC.setPins(SD_SCK, SD_MOSI /* CMD */, SD_MISO /* D0 */);
  if (!SD_MMC.begin(root, true /* mode1bit */, false /* format_if_mount_failed */, SDMMC_FREQ_DEFAULT))
  {
    Serial.println("ERROR: SD Card mount failed!");
    while (true)
    {

    }
  }
  else{Serial.println("ja...");
        }