#include <sleepy_discord/voice_connection.h>

#include "opus_decoder.h"

SleepyDiscord::CustomInitOpusDecoder SleepyDiscord::CustomOpusDecoder::init =
    []() -> SleepyDiscord::GenericOpusDecoder* { return new OpusDecoder; };

OpusDecoder::OpusDecoder() {
  printf("Init opus decoder\n");
  Result res = hwopusDecoderInitialize(
      &hwdecoder, SleepyDiscord::AudioTransmissionDetails::bitrate(),
      SleepyDiscord::AudioTransmissionDetails::channels());
  if (R_FAILED(res)) {
    printf("hwopusDecoderInitialize: %08" PRIx32 "\n", res);
  }
}

OpusDecoder::~OpusDecoder() {
  printf("Close opus decoder\n");
  hwopusDecoderExit(&hwdecoder);
}

int OpusDecoder::decodeOpus(uint8_t* encodedData, size_t encodedDataSize,
                            int16_t* decodedData) {
  int decoded_data_size;
  int decoded_sample_count;
  HwopusHeader header = {0};
  header.size = __builtin_bswap32(encodedDataSize);
  std::memcpy(
      encodedData - sizeof(HwopusHeader), &header,
      sizeof(HwopusHeader));  // encodedData should have 12 bytes prepended RTP
                              // header, HwopusHeader is 8 bytes

  Result res = hwopusDecodeInterleaved(
      &hwdecoder, &decoded_data_size, &decoded_sample_count,
      encodedData - sizeof(HwopusHeader),
      encodedDataSize + sizeof(HwopusHeader), decodedData,
      SleepyDiscord::AudioTransmissionDetails::proposedLength() *
          sizeof(int16_t));
  if (R_FAILED(res)) {
    printf("Opus decoding error: %08" PRIx32 "\n", res);
    return -1;
  }
  return 0;
}
