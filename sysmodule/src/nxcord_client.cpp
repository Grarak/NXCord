#include <malloc.h>

#include "nxcord_client.h"

class AudioReceiver : public SleepyDiscord::BaseAudioOutput {
 private:
  AudioOutBuffer _audout_buf;

 public:
  AudioReceiver() : BaseAudioOutput() {
    Result rc = audoutStartAudioOut();
    printf("audoutStartAudioOut() returned 0x%x\n", rc);

    size_t buffer_size = (opus_framesize_bytes + 0xfff) & ~0xfff;
    void* out_buf_data = memalign(0x1000, buffer_size);
    std::memset(out_buf_data, 0, buffer_size);

    _audout_buf.next = nullptr;
    _audout_buf.buffer = out_buf_data;
    _audout_buf.buffer_size = buffer_size;
    _audout_buf.data_size = opus_framesize_bytes;
    _audout_buf.data_offset = 0;

    rc = audoutAppendAudioOutBuffer(&_audout_buf);
    printf("audoutAppendAudioOutBuffer() returned 0x%x\n", rc);
  }

  void write(Container audio,
             SleepyDiscord::AudioTransmissionDetails& details) override {
    uint32_t released_out_count = 0;
    AudioOutBuffer* audout_released_buf = nullptr;

    audoutWaitPlayFinish(&audout_released_buf, &released_out_count, U64_MAX);
    if (released_out_count > 0) {
      if (audout_released_buf) {
        std::memcpy(audout_released_buf->buffer, audio.data(),
                    audout_released_buf->data_size);
      }
      audoutAppendAudioOutBuffer(audout_released_buf);
    }
  }

  ~AudioReceiver() {
    printf("Releasing audio receiver\n");
    uint32_t released_out_count = 0;
    AudioOutBuffer* audout_released_buf = nullptr;
    audoutWaitPlayFinish(&audout_released_buf, &released_out_count, U64_MAX);
    Result rc = audoutStopAudioOut();
    printf("audoutStopAudioOut() returned 0x%x\n", rc);
  }
};

struct AudioSender : public SleepyDiscord::AudioVectorSource {
 private:
  // AudioInBuffer _audin_buf;

 public:
  /*AudioSender() {
    Result rc = audinStartAudioIn();
    printf("audinStartAudioIn() returned 0x%x\n", rc);

    u32 buffer_size = (opus_framesize_bytes + 0xfff) & ~0xfff;
    void* in_buf_data = memalign(0x1000, buffer_size);
    std::memset(in_buf_data, 0, buffer_size);

    _audin_buf.next = NULL;
    _audin_buf.buffer = in_buf_data;
    _audin_buf.buffer_size = buffer_size;
    _audin_buf.data_size = opus_framesize_bytes;
    _audin_buf.data_offset = 0;

    rc = audinAppendAudioInBuffer(&_audin_buf);
    printf("audinAppendAudioInBuffer() returned 0x%x\n", rc);
}*/
  void read(SleepyDiscord::AudioTransmissionDetails& details,
            SleepyDiscord::AudioVectorSource::Container& target) override {
    /*uint32_t released_in_count;
    AudioInBuffer* audin_released_buf = nullptr;

    audinWaitCaptureFinish(&audin_released_buf, &released_in_count, 200);
    if (released_in_count > 0) {
      if (audin_released_buf) {
        std::memcpy(target.data(), audin_released_buf->buffer,
                    audin_released_buf->data_size);
      }
      audinAppendAudioInBuffer(audin_released_buf);
  }*/
  }

  /*~AudioSender() {
    printf("Releasing audio sender\n");
    uint32_t released_in_count;
    AudioInBuffer* audin_released_buf = nullptr;
    audinWaitCaptureFinish(&audin_released_buf, &released_in_count, 200);
    Result rc = audinStopAudioIn();
    printf("audinStopAudioIn() returned 0x%x\n", rc);
}*/
};

class VoiceEventHandler : public SleepyDiscord::BaseVoiceEventHandler {
 public:
  void onReady(SleepyDiscord::VoiceConnection& connection) override {
    SleepyDiscord::BaseAudioOutput* audio_receiver = new AudioReceiver;
    connection.setAudioOutput(audio_receiver);
    connection
        .startSpeaking<AudioSender>();  // Send garbage to discord, otherwise we
                                        // won't receive audio
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

NXCordClient::NXCordClient(const std::string& token) : DiscordClient(token) {}

void NXCordClient::onReady(SleepyDiscord::Ready readyData) {
  printf("Joining voice channel\n");
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
        _current_voice_context = &context;
        context.startVoiceHandler<VoiceEventHandler>();
        break;
      }
    }
  }
}

void NXCordClient::onResumed() {
  if (_current_voice_context) {
    printf("Rejoining voice channel %ld %ld\n",
           _current_voice_context->getChannelID().number(),
           _current_voice_context->getServerID().number());
    connectToVoiceChannel(*_current_voice_context);
  }
}

void NXCordClient::onMessage(SleepyDiscord::Message message) {}
