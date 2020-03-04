#include <common/logger.hpp>
#include <common/utils.hpp>
#include <nxcord/discord_websocket.hpp>
#include <nxcord/nxcord_audio_capture.hpp>
#include <nxcord/nxcord_audio_player.hpp>
#include <nxcord/nxcord_client.hpp>

class AudioReceiver : public SleepyDiscord::BaseAudioOutput {
 private:
  NXCordClient &_client;
  NXCordAudioPlayer _player;

  friend NXCordClient;

 public:
  explicit AudioReceiver(NXCordClient &client) : _client(client) {}

  void write(std::vector<SleepyDiscord::AudioSample> &audio,
             SleepyDiscord::AudioTransmissionDetails &details) override {
    std::string global_audio_volume_str =
        _client.getSettings().getvoiceglobal_audio_volume();
    float global_audio_volume = std::stof(global_audio_volume_str);
    for (auto &sample : audio) {
      sample *= global_audio_volume;
    }
    _player.queue(audio, details);
  }
};

class AudioSender : public SleepyDiscord::AudioVectorSource {
 private:
  NXCordClient &_client;
  NXCordAudioCapture _audio_capture;
  std::vector<SleepyDiscord::AudioSample> _capture;

 public:
  explicit AudioSender(NXCordClient &client) : _client(client) {}

  ~AudioSender() override { _client._previous_mic_volume = 0; }

  bool frameAvailable() override {
    _capture = _audio_capture.poll();
    float average_volume = 0;
    std::string multiplier_str = _client.getSettings().getvoicemic_multiplier();
    size_t multiplier = std::stof(multiplier_str);
    if (!_capture.empty()) {
      for (auto &sample : _capture) {
        sample *= multiplier;
        average_volume += static_cast<float>(std::abs(sample)) /
                          static_cast<float>(Utils::audio_sample_max);
      }
      average_volume /= static_cast<float>(_capture.size());
    }
    _client._previous_mic_volume = average_volume;

    std::string threshold_str = _client.getSettings().getvoicemic_threshold();
    float threshold = std::stof(threshold_str);
    return average_volume > std::max(threshold, 0.00001f);
  }

  void read(SleepyDiscord::AudioTransmissionDetails &details,
            SleepyDiscord::AudioVectorSource::Container &target) override {
    if (!_capture.empty()) {
      std::memcpy(target.data(), _capture.data(),
                  _capture.size() * sizeof(SleepyDiscord::AudioSample));
    }
  }
};

class VoiceEventHandler : public SleepyDiscord::BaseVoiceEventHandler {
 private:
  NXCordClient &_client;

 public:
  explicit VoiceEventHandler(NXCordClient &client) : _client(client) {}

  void onReady(SleepyDiscord::VoiceConnection &connection) override {
    Logger::write("NXCordClient: VoiceEventHandler: onReady\n");

    _client._current_voice_connection = &connection;
    _client._current_voice_context =
        NXCordClient::LocalVoiceContext(connection.getContext());

    SleepyDiscord::BaseAudioOutput *audio_receiver = new AudioReceiver(_client);
    connection.setAudioOutput(audio_receiver);
    connection.startSpeaking<AudioSender>(_client);
    connection.startListening();
  }

  inline void onHeartbeat(SleepyDiscord::VoiceConnection &) override {
    Logger::write("Voice heartbeat sent\n");
  }

  inline void onHeartbeatAck(SleepyDiscord::VoiceConnection &,
                             const time_t ping) override {
    Logger::write("Voice heartbeat acknowledged ping %lums\n", ping);
  }
};

void NXCordClient::onReady() {
  Logger::write("NXCordClient: onReady\n");
  _ready = true;
  _servers.clear();
  _servers.shrink_to_fit();
  _channels.clear();
  _current_voice_connection = nullptr;
  _voice_states.clear();

  std::vector<SleepyDiscord::Server> servers = getServers().vector();
  for (const auto &server : servers) {
    addServer(server);
  }
}

void NXCordClient::onResumed() {
  Logger::write("NXCordClient: onResume\n");
  _ready = true;
  _channels.clear();

  _current_voice_connection = nullptr;
  if (!_current_voice_context.sessionID.empty() &&
      !_current_voice_context.endpoint.empty()) {
    Logger::write("Rejoining voice channel %ld %ld\n",
                  _current_voice_context.serverID.number(),
                  _current_voice_context.channelID.number());
    SleepyDiscord::VoiceContext &context = createVoiceContext(
        _current_voice_context.serverID, _current_voice_context.channelID);
    context.sessionID = _current_voice_context.sessionID;
    context.endpoint = _current_voice_context.endpoint;
    context.token = _current_voice_context.token;
    context.startVoiceHandler<VoiceEventHandler>(*this);
    connectToVoiceChannel(context);
  }
}

void NXCordClient::onServer(SleepyDiscord::Server server) {
  Logger::write("NXCordClient: onServer %s %ld\n", server.name.c_str(),
                server.ID.number());
  addServer(server);

  for (const auto &voice_state : server.voiceStates) {
    if (!voice_state.channelID.string().empty()) {
      auto it = server.findMember(voice_state.userID);
      if (it != server.members.end()) {
        std::vector<IPCStruct::DiscordVoiceState> &states =
            _voice_states[voice_state.channelID.number()];

        std::string &name = it->nick;
        if (name.empty()) {
          name = it->user.username;
        }
        states.emplace_back(IPCStruct::create_discord_voice_state(
            voice_state.userID.number(), name));
      }
    }
  }
}

void NXCordClient::onEditServer(SleepyDiscord::Server editServer) {
  Logger::write("NXCordClient: onEditServer %s %ld\n", editServer.name.c_str(),
                editServer.ID.number());

  auto it = std::find_if(_servers.begin(), _servers.end(),
                         [&editServer](const IPCStruct::DiscordServer &server) {
                           return editServer.ID.number() == server.id;
                         });
  if (it != _servers.end()) {
    IPCStruct::DiscordServer discord_server = IPCStruct::create_discord_server(
        editServer.name, editServer.ID.number());
    *it = discord_server;
  }
}

void NXCordClient::onDeleteServer(
    SleepyDiscord::UnavailableServer unavailableServer) {
  Logger::write("NXCordClient: onDeleteServer %ld\n",
                unavailableServer.ID.number());

  _servers.erase(
      std::remove_if(
          _servers.begin(), _servers.end(),
          [&unavailableServer](const IPCStruct::DiscordServer &server) {
            return unavailableServer.ID.number() == server.id;
          }),
      _servers.end());
}

void NXCordClient::addServer(const SleepyDiscord::Server &serverToAdd) {
  auto it =
      std::find_if(_servers.begin(), _servers.end(),
                   [&serverToAdd](const IPCStruct::DiscordServer &server) {
                     return serverToAdd.ID.number() == server.id;
                   });
  if (it != _servers.end()) {
    return;
  }

  IPCStruct::DiscordServer discord_server{};
  size_t name_size =
      std::min(sizeof(discord_server.name) - 1, serverToAdd.name.size());
  std::memcpy(discord_server.name, serverToAdd.name.c_str(), name_size);
  discord_server.name[name_size] = '\0';
  discord_server.id = serverToAdd.ID.number();
  _servers.push_back(discord_server);
}

inline IPCStruct::DiscordChannel create_ipc_channel(
    const SleepyDiscord::Channel &channel) {
  return IPCStruct::create_discord_channel(
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
        [&channel](const IPCStruct::DiscordChannel &cached_channel) {
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
            [&channel](const IPCStruct::DiscordChannel &cached_channel) {
              return channel.ID.number() == cached_channel.id;
            }),
        channels->end());
  } else {
    fillChannels(channel.serverID);
  }
}

void NXCordClient::fillChannels(
    const SleepyDiscord::Snowflake<SleepyDiscord::Server> &channel) {
  std::vector<IPCStruct::DiscordChannel> &cached_channels =
      _channels[channel.number()];
  cached_channels.clear();
  std::vector<SleepyDiscord::Channel> channels =
      getServerChannels(channel).vector();
  for (const auto &current_channels : channels) {
    cached_channels.push_back(create_ipc_channel(current_channels));
  }
  cached_channels.shrink_to_fit();
}

const std::vector<IPCStruct::DiscordChannel> &NXCordClient::getCachedChannels(
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

void NXCordClient::onEditVoiceState(const SleepyDiscord::VoiceState &state) {
  for (auto &pair : _voice_states) {
    std::vector<IPCStruct::DiscordVoiceState> &states = pair.second;
    auto it = std::find_if(
        states.begin(), states.end(),
        [&state](const IPCStruct::DiscordVoiceState &discordVoiceState) {
          return state.userID.number() == discordVoiceState.userId;
        });
    if (it != states.end()) {
      states.erase(it);
      if (states.empty()) {
        _voice_states.erase(pair.first);
      }
      break;
    }
  }

  if (!state.channelID.string().empty()) {
    std::vector<IPCStruct::DiscordVoiceState> &states =
        _voice_states[state.channelID.number()];
    std::string name = state.serverMember.nick;
    if (name.empty()) {
      name = state.serverMember.user.username;
    }

    states.emplace_back(
        IPCStruct::create_discord_voice_state(state.userID.number(), name));
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

  SleepyDiscord::VoiceContext &context = connectToVoiceChannel(server, channel);
  context.startVoiceHandler<VoiceEventHandler>(*this);
}

void NXCordClient::disconnectVoiceChannel() {
  if (_current_voice_connection) {
    removeVoiceConnectionAndContext(*_current_voice_connection);
    _current_voice_connection = nullptr;
    _current_voice_context = LocalVoiceContext();
  }
}

IPCStruct::DiscordServer NXCordClient::getServer(int64_t serverId) const {
  auto it = std::find_if(_servers.begin(), _servers.end(),
                         [&serverId](const IPCStruct::DiscordServer &server) {
                           return serverId == server.id;
                         });
  if (it != _servers.end()) {
    return *it;
  }
  return IPCStruct::DiscordServer();
}

IPCStruct::DiscordChannel NXCordClient::getConnectedVoiceChannel() const {
  if (isConnectedVoiceChannel()) {
    for (const auto &pair : _channels) {
      const std::vector<IPCStruct::DiscordChannel> &channel = pair.second;
      auto it = std::find_if(
          channel.begin(), channel.end(),
          [this](const IPCStruct::DiscordChannel &channel) {
            return _current_voice_context.channelID.number() == channel.id;
          });
      if (it != channel.end()) {
        return *it;
      }
    }
  }
  return IPCStruct::DiscordChannel();
}

void NXCordClient::loadSettings(std::unique_ptr<NXCordSettings> &settings) {
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

void NXCordClient::setVoiceUserMultiplier(int64_t user_id, float multiplier) {
  if (isConnectedVoiceChannel()) {
    const std::map<int64_t, uint32_t> &ssrcs =
        _current_voice_connection->getUserSSRCs();
    auto it = ssrcs.find(user_id);
    if (it != ssrcs.end()) {
      auto &audio_receiver = dynamic_cast<AudioReceiver &>(
          _current_voice_connection->getAudioOutput());
      audio_receiver._player.setSSRCMultiplier(it->second, multiplier);
    }
  }
}

float NXCordClient::getVoiceUserMultiplier(int64_t user_id) const {
  if (isConnectedVoiceChannel()) {
    const std::map<int64_t, uint32_t> &ssrcs =
        _current_voice_connection->getUserSSRCs();
    auto it = ssrcs.find(user_id);
    if (it != ssrcs.end()) {
      auto &audio_receiver = dynamic_cast<AudioReceiver &>(
          _current_voice_connection->getAudioOutput());
      return audio_receiver._player.getSSRCMultiplier(it->second);
    }
  }
  return 1;
}
