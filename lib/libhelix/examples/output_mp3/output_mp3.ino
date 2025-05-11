#include <AudioTools.h>
#include <MP3DecoderHelix.h>
#include <SD_MMC.h>

using namespace audio_tools;
using namespace libhelix;

MP3DecoderHelix decoder;
I2SStream i2s;

void setup() {
  Serial.begin(115200);

  if (!SD_MMC.begin()) {
    Serial.println("SD_MMC Mount Failed");
    return;
  }

  File file = SD_MMC.open("/sound.mp3");
  if (!file) {
    Serial.println("Datei nicht gefunden");
    return;
  }

  auto cfg = i2s.defaultConfig();
  cfg.sample_rate = 44100;
  cfg.bits_per_sample = 16;
  cfg.channels = 2;
  i2s.begin(cfg);

  decoder.begin();

  uint8_t buf[512];
  while (file.available()) {
    size_t bytesRead = file.read(buf, sizeof(buf));
    decoder.write(buf, bytesRead, [&i2s](MP3FrameInfo &info, int16_t *pcm, size_t len, void *) {
i2s.write((uint8_t*)pcm, len * sizeof(int16_t));
    });
  }

  file.close();
}

void loop() {}
