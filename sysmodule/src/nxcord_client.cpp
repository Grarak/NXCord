#include "logger.h"
#include "nxcord_audio_capture.h"
#include "nxcord_audio_player.h"
#include "nxcord_client.h"
#include "utils.h"

class AudioReceiver : public SleepyDiscord::BaseAudioOutput {
 private:
  NXCordAudioPlayer _player;

 public:
  void write(std::vector<SleepyDiscord::AudioSample>& audio,
             SleepyDiscord::AudioTransmissionDetails& details) override {
    _player.queue(audio, details);
  }
};

struct AudioSender : public SleepyDiscord::AudioVectorSource {
 private:
  NXCordAudioCapture _audio_capture;
  std::vector<SleepyDiscord::AudioSample> _capture;

 public:
  bool frameAvailable() {
    _capture = std::move(_audio_capture.poll());
    float average_volume = 0;
    if (_capture.size() > 0) {
      for (const auto& sample : _capture) {
        average_volume = static_cast<float>(std::abs(sample)) /
                         static_cast<float>(Utils::audio_sample_max);
      }
      average_volume /= static_cast<float>(_capture.size());
    }
    return average_volume > 0.00001;  // Set user defined threshold here
  }

  void read(SleepyDiscord::AudioTransmissionDetails& details,
            SleepyDiscord::AudioVectorSource::Container& target) override {
    if (_capture.size() > 0) {
      std::memcpy(target.data(), _capture.data(),
                  _capture.size() * sizeof(SleepyDiscord::AudioSample));
    }
  }
};

class VoiceEventHandler : public SleepyDiscord::BaseVoiceEventHandler {
 private:
 public:
  void onReady(SleepyDiscord::VoiceConnection& connection) override {
    SleepyDiscord::BaseAudioOutput* audio_receiver = new AudioReceiver();
    connection.setAudioOutput(audio_receiver);
    connection
        .startSpeaking<AudioSender>();  // Send garbage to discord, otherwise we
                                        // won't receive audio
    connection.startListening();
  }

  void onHeartbeat(SleepyDiscord::VoiceConnection&) override {
    Logger::write("Voice heartbeat sent\n");
  }

  void onHeartbeatAck(SleepyDiscord::VoiceConnection&,
                      const time_t ping) override {
    Logger::write("Voice heartbeat acknowledged ping %lums\n", ping);
  }
};

NXCordClient::NXCordClient(const std::string& token) : DiscordClient(token) {}

void NXCordClient::onReady(SleepyDiscord::Ready readyData) {
  Logger::write("Joining voice channel\n");
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
    Logger::write("Rejoining voice channel %ld %ld\n",
                  _current_voice_context->getChannelID().number(),
                  _current_voice_context->getServerID().number());
    connectToVoiceChannel(*_current_voice_context);
  }
}

void NXCordClient::onMessage(SleepyDiscord::Message message) {}
