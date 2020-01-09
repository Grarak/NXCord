#pragma once

#include <sleepy_discord/custom_opus_decoder.h>
#include <switch.h>

class OpusDecoder : public SleepyDiscord::GenericOpusDecoder {
 private:
  HwopusDecoder _hwdecoder = {0};
  bool _dummy;

 public:
  OpusDecoder(bool dummy = false);
  ~OpusDecoder() override;
  int decodeOpus(uint8_t* encodedData, size_t encodedDataSize,
                 int16_t* decodedData) override;
};
