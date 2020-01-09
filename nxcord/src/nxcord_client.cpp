#include <common/logger.hpp>
#include <common/utils.hpp>
#include <nxcord/nxcord_audio_capture.hpp>
#include <nxcord/nxcord_audio_player.hpp>
#include <nxcord/nxcord_client.hpp>

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
  NXCordClient* _client;

 public:
  VoiceEventHandler(NXCordClient* client) : _client(client) {}

  void onReady(SleepyDiscord::VoiceConnection& connection) override {
    Logger::write("NXCordClient: VoiceEventHandler: onReady\n");

    _client->_current_voice_connection = &connection;
    _client->_current_voice_context = connection.getContext();

    SleepyDiscord::BaseAudioOutput* audio_receiver = new AudioReceiver();
    connection.setAudioOutput(audio_receiver);
    connection.startSpeaking<AudioSender>();
    connection.startListening();
  }

  inline void onHeartbeat(SleepyDiscord::VoiceConnection&) override {
    Logger::write("Voice heartbeat sent\n");
  }

  inline void onHeartbeatAck(SleepyDiscord::VoiceConnection&,
                             const time_t ping) override {
    Logger::write("Voice heartbeat acknowledged ping %lums\n", ping);
  }
};

void NXCordClient::onReady(SleepyDiscord::Ready readyData) {
  Logger::write("NXCordClient: onReady\n");
  _ready = true;
  _servers.clear();
  _channels.clear();
  _current_voice_connection = nullptr;

  std::vector<SleepyDiscord::Server> servers = getServers().vector();
  for (const auto& server : servers) {
    addServer(server);
  }
}

void NXCordClient::onResumed() {
  Logger::write("NXCordClient: onResume\n");
  _channels.clear();

  _current_voice_connection = nullptr;
  if (!_current_voice_context.sessionID.empty() &&
      !_current_voice_context.endpoint.empty()) {
    Logger::write("Rejoining voice channel %ld %ld\n",
                  _current_voice_context.serverID.number(),
                  _current_voice_context.channelID.number());
    SleepyDiscord::VoiceContext& context = createVoiceContext(
        _current_voice_context.serverID, _current_voice_context.channelID);
    context.sessionID = _current_voice_context.sessionID;
    context.endpoint = _current_voice_context.endpoint;
    context.token = _current_voice_context.token;
    context.startVoiceHandler<VoiceEventHandler>(this);
    connectToVoiceChannel(context);
  }
}

void NXCordClient::onServer(SleepyDiscord::Server server) {
  Logger::write("NXCordClient: onServer %s %ld\n", server.name.c_str(),
                server.ID.number());
  addServer(server);
}

void NXCordClient::onEditServer(SleepyDiscord::Server editServer) {
  Logger::write("NXCordClient: onEditServer %s %ld\n", editServer.name.c_str(),
                editServer.ID.number());

  auto it = std::find_if(_servers.begin(), _servers.end(),
                         [&editServer](const IPCStruct::DiscordServer& server) {
                           return editServer.ID.number() == server.id;
                         });
  if (it != _servers.end()) {
    IPCStruct::DiscordServer discord_server =
        NXCordComInterface::create_discord_server(editServer.name,
                                                  editServer.ID.number());
    *it = std::move(discord_server);
  }
}

void NXCordClient::onDeleteServer(
    SleepyDiscord::UnavailableServer unavailableServer) {
  Logger::write("NXCordClient: onDeleteServer %ld\n",
                unavailableServer.ID.number());

  _servers.erase(
      std::remove_if(
          _servers.begin(), _servers.end(),
          [&unavailableServer](const IPCStruct::DiscordServer& server) {
            return unavailableServer.ID.number() == server.id;
          }),
      _servers.end());
}

void NXCordClient::addServer(const SleepyDiscord::Server& serverToAdd) {
  auto it =
      std::find_if(_servers.begin(), _servers.end(),
                   [&serverToAdd](const IPCStruct::DiscordServer& server) {
                     return serverToAdd.ID.number() == server.id;
                   });
  if (it != _servers.end()) {
    return;
  }

  IPCStruct::DiscordServer discord_server;
  size_t name_size =
      std::min(sizeof(discord_server.name) - 1, serverToAdd.name.size());
  std::memcpy(discord_server.name, serverToAdd.name.c_str(), name_size);
  discord_server.name[name_size] = '\0';
  discord_server.id = serverToAdd.ID.number();
  _servers.push_back(std::move(discord_server));
}

inline IPCStruct::DiscordChannel create_ipc_channel(
    const SleepyDiscord::Channel& channel) {
  return NXCordComInterface::create_discord_channel(
      channel.name, channel.serverID.number(), channel.ID.number(),
      static_cast<IPCStruct::DiscordChannelType>(channel.type));
}

void NXCordClient::onChannel(SleepyDiscord::Channel channel) {
  Logger::write("NXCordClient: onChannel %s %ld\n", channel.name.c_str(),
                channel.ID.number());

  auto channels = getChannelsPtr(channel);
  if (channels) {
    channels->push_back(create_ipc_channel(channel));
  } else {
    fillChannels(channel.serverID);
  }
}

void NXCordClient::onEditChannel(SleepyDiscord::Channel channel) {
  Logger::write("NXCordClient: onEditChannel %s %ld\n", channel.name.c_str(),
                channel.ID.number());

  auto channels = getChannelsPtr(channel);
  if (channels) {
    auto it = std::find_if(
        channels->begin(), channels->end(),
        [&channel](const IPCStruct::DiscordChannel& cached_channel) {
          return channel.ID.number() == cached_channel.id;
        });

    if (it != channels->end()) {
      IPCStruct::DiscordChannel new_channel = create_ipc_channel(channel);
      *it = new_channel;
    }
  } else {
    fillChannels(channel.serverID);
  }
}

void NXCordClient::onDeleteChannel(SleepyDiscord::Channel channel) {
  Logger::write("NXCordClient: onDeleteChannel %s %ld\n", channel.name.c_str(),
                channel.ID.number());

  auto channels = getChannelsPtr(channel);
  if (channels) {
    channels->erase(
        std::remove_if(
            channels->begin(), channels->end(),
            [&channel](const IPCStruct::DiscordChannel& cached_channel) {
              return channel.ID.number() == cached_channel.id;
            }),
        channels->end());
  } else {
    fillChannels(channel.serverID);
  }
}

void NXCordClient::fillChannels(
    const SleepyDiscord::Snowflake<SleepyDiscord::Server>& channel) {
  std::vector<IPCStruct::DiscordChannel>& cached_channels =
      _channels[channel.number()];
  cached_channels.clear();
  std::vector<SleepyDiscord::Channel> channels =
      getServerChannels(channel).vector();
  for (const auto& channel : channels) {
    cached_channels.push_back(create_ipc_channel(channel));
  }
}

const std::vector<IPCStruct::DiscordChannel>& NXCordClient::getCachedChannels(
    int64_t serverId) {
  SleepyDiscord::Snowflake<SleepyDiscord::Server> serverSnowflake(serverId);
  auto channels = getChannelsPtr(serverSnowflake);
  if (channels) {
    return *channels;
  } else {
    fillChannels(serverSnowflake);
    return _channels[serverId];
  }
}

void NXCordClient::joinVoiceChannel(int64_t serverId, int64_t channelId) {
  Logger::write("Joining voice channel %ld %ld\n", serverId, channelId);

  if (isConnectedVoiceChannel()) {
    Logger::write("Leaving previous channel\n");
    disconnectVoiceChannel();
  }

  SleepyDiscord::Snowflake<SleepyDiscord::Server> server(serverId);
  SleepyDiscord::Snowflake<SleepyDiscord::Channel> channel(channelId);

  SleepyDiscord::VoiceContext& context = connectToVoiceChannel(server, channel);
  context.startVoiceHandler<VoiceEventHandler>(this);
}

bool NXCordClient::isConnectedVoiceChannel() {
  return _current_voice_connection != nullptr;
}

void NXCordClient::disconnectVoiceChannel() {
  if (_current_voice_connection) {
    removeVoiceConnectionAndContext(*_current_voice_connection);
    _current_voice_connection = nullptr;
    _current_voice_context = LocalVoiceContext();
  }
}

void NXCordClient::loadSettings(std::unique_ptr<NXCordSettings>& settings) {
  if (settings->valid()) {
    _settings = std::move(settings);
    _token = _settings->gettoken();
  }
}

void NXCordClient::startConnection() {
  if (_settings) {
    _settings->settoken(_token);
  }
  DiscordClient::startConnection();
}
