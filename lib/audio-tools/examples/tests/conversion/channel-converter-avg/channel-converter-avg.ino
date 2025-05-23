/**
 * @file channel-converter-avg.ino
 * @brief Test calculating pairwise average of channels
 * @author Urs Utzinger
 * @copyright GPLv3
 **/
 
#include "AudioTools.h"

AudioInfo                     info1(44100, 1, 16);
AudioInfo                     info2(44100, 2, 16);
SineWaveGenerator<int16_t>    sineWave1(32000);                    // subclass of SoundGenerator with max amplitude of 32000
SineWaveGenerator<int16_t>    sineWave2(32000);                    // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> sound1(sineWave1);                   // stream generated from sine wave1
GeneratedSoundStream<int16_t> sound2(sineWave2);                   // stream generated from sine wave2
InputMerge<int16_t>           imerge;                              // merge two inputs to stereo
ChannelAvg                    averager;                            // channel averager
ConverterStream<int16_t>      averaged_stream(imerge, averager);   // pipe the sound to the averager
CsvOutput<int16_t>            serial_out(Serial);                  // serial output
StreamCopy                    copier(serial_out, averaged_stream); // stream the binner output to serial port

// Arduino Setup
void setup(void) {

  // Open Serial 
  Serial.begin(115200);
  while(!Serial); // wait for Serial to be ready

  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);

  // Setup sine waves
  sineWave1.begin(info1, N_B4);
  sineWave2.begin(info1, N_B5);

  // Merge input to stereo
  imerge.add(sound1, 1);
  imerge.add(sound2, 1);
  imerge.begin(info2);

  // Define CSV Output
  serial_out.begin(info1);

}  

// Arduino loop - copy sound to out with conversion
void loop() {
  copier.copy();
}