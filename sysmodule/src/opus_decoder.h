#pragma once

#include <sleepy_discord/custom_opus_decoder.h>
#include <switch.h>

class OpusDecoder : public SleepyDiscord::GenericOpusDecoder {
 private:
  HwopusDecoder hwdecoder = {0};

 public:
  OpusDecoder();
  ~OpusDecoder() override;
  int decodeOpus(uint8_t* encodedData, size_t encodedDataSize,
                 int16_t* decodedData) override;
};
