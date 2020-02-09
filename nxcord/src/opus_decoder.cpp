#include <sleepy_discord/voice_connection.h>

#include <common/logger.hpp>
#include <nxcord/opus_decoder.hpp>

SleepyDiscord::CustomInitOpusDecoder SleepyDiscord::CustomOpusDecoder::init =
    []() -> SleepyDiscord::GenericOpusDecoder * { return new OpusDecoder; };

OpusDecoder::OpusDecoder(bool dummy) : _dummy(dummy) {
  if (!_dummy) {
    Logger::write("Init opus decoder\n");
    Result res = hwopusDecoderInitialize(
        &_hwdecoder, SleepyDiscord::AudioTransmissionDetails::bitrate(),
        SleepyDiscord::AudioTransmissionDetails::channels());
    if (R_FAILED(res)) {
      Logger::write("hwopusDecoderInitialize: %08" PRIx32 "\n", res);
    }
  }
}

OpusDecoder::~OpusDecoder() {
  if (!_dummy) {
    Logger::write("Close opus decoder\n");
    hwopusDecoderExit(&_hwdecoder);
  }
}

int OpusDecoder::decodeOpus(uint8_t *encodedData, size_t encodedDataSize,
                            int16_t *decodedData) {
  int decoded_data_size;
  int decoded_sample_count;
  HwopusHeader header = {0};
  header.size = __builtin_bswap32(encodedDataSize);
  std::memcpy(
      encodedData - sizeof(HwopusHeader), &header,
      sizeof(HwopusHeader));  // encodedData should have 12 bytes prepended RTP
  // header, HwopusHeader is 8 bytes

  Result res = hwopusDecodeInterleaved(
      &_hwdecoder, &decoded_data_size, &decoded_sample_count,
      encodedData - sizeof(HwopusHeader),
      encodedDataSize + sizeof(HwopusHeader), decodedData,
      SleepyDiscord::AudioTransmissionDetails::proposedLength() *
          sizeof(SleepyDiscord::AudioSample));
  if (R_FAILED(res)) {
    Logger::write("Opus decoding error: %08" PRIx32 "\n", res);
    return -1;
  }
  return decoded_sample_count;
}
