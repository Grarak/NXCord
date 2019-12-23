#include <malloc.h>

#include "nxcord_client.h"

class AudioReceiver : public SleepyDiscord::BaseAudioOutput {
 private:
  static constexpr size_t framesize_bytes =
      SleepyDiscord::AudioTransmissionDetails::proposedLength() *
      sizeof(SleepyDiscord::AudioSample);
  AudioOutBuffer _audout_buf;

 public:
  AudioReceiver() : BaseAudioOutput() {
    size_t buffer_size = (framesize_bytes + 0xfff) & ~0xfff;
    void* out_buf_data = memalign(0x1000, buffer_size);
    std::memset(out_buf_data, 0, buffer_size);

    _audout_buf.next = nullptr;
    _audout_buf.buffer = out_buf_data;
    _audout_buf.buffer_size = buffer_size;
    _audout_buf.data_size = framesize_bytes;
    _audout_buf.data_offset = 0;

    Result rc = audoutAppendAudioOutBuffer(&_audout_buf);
    printf("audoutAppendAudioOutBuffer() returned 0x%x\n", rc);
  }

  void write(Container audio,
             SleepyDiscord::AudioTransmissionDetails& details) override {
    uint32_t released_out_count;
    AudioOutBuffer* audout_released_buf = nullptr;

    audoutWaitPlayFinish(&audout_released_buf, &released_out_count, U64_MAX);
    if (audout_released_buf) {
      std::memcpy(audout_released_buf->buffer, audio.data(), framesize_bytes);
    }
    audoutAppendAudioOutBuffer(audout_released_buf);
  }
};

struct SquareWave : public SleepyDiscord::AudioVectorSource {
  using SleepyDiscord::AudioVectorSource::AudioVectorSource;
  void read(SleepyDiscord::AudioTransmissionDetails& details,
            SleepyDiscord::AudioVectorSource::Container& target) override {
    for (SleepyDiscord::AudioSample& sample : target) {
      sample = (++sampleOffset / 100) % 2 ? volume : -1 * volume;
    }
  }
  std::size_t sampleOffset = 0;
  int volume = 2000;
  int halfSquareWaveLength = 100;
};

class VoiceEventHandler : public SleepyDiscord::BaseVoiceEventHandler {
 public:
  void onReady(SleepyDiscord::VoiceConnection& connection) override {
    SleepyDiscord::BaseAudioOutput* audio_receiver = new AudioReceiver;
    connection.setAudioOutput(audio_receiver);
    connection.startSpeaking<SquareWave>();  // Send garbage to discord,
                                             // otherwise we won't receive audio
    connection.startListening();
  }

  void onHeartbeat(SleepyDiscord::VoiceConnection&) override {
    printf("Voice heartbeat sent\n");
  }

  void onHeartbeatAck(SleepyDiscord::VoiceConnection&,
                      const time_t ping) override {
    printf("Voice heartbeat acknowledged ping %lums\n", ping);
  }
};

NXCordClient::NXCordClient(const std::string& token) : DiscordClient(token) {
  Result rc = audoutStartAudioOut();
  printf("audoutStartAudioOut() returned 0x%x\n", rc);
}

NXCordClient::~NXCordClient() {
  Result rc = audoutStopAudioOut();
  printf("audoutStopAudioOut() returned 0x%x\n", rc);
}

void NXCordClient::onReady(SleepyDiscord::Ready readyData) {
  std::vector<SleepyDiscord::Server> servers = getServers().vector();
  // Join the fist voice channel we can find
  if (!servers.empty()) {
    SleepyDiscord::Server& server = servers[0];
    std::vector<SleepyDiscord::Channel> channels =
        getServerChannels(server).vector();
    for (const auto& channel : channels) {
      if (channel.type == SleepyDiscord::Channel::ChannelType::SERVER_VOICE) {
        SleepyDiscord::VoiceContext& context =
            connectToVoiceChannel(server, channel);
        context.startVoiceHandler<VoiceEventHandler>();
        break;
      }
    }
  }
}

void NXCordClient::onMessage(SleepyDiscord::Message message) {}
