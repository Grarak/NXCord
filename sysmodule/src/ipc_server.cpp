#include "ipc_server.hpp"

IPCServer *IPCServer::instance = nullptr;

ams::Result NXCordService::IsConnected(const ams::sf::OutBuffer &out_path) {
  IPCServer::instance->executeFunction([&out_path](NXCordClient &client) {
    setOut<bool>(out_path, client.isConnected());
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::IsConnecting(const ams::sf::OutBuffer &out_path) {
  IPCServer::instance->executeFunction([&out_path](NXCordClient &client) {
    setOut<bool>(out_path, client.isConnecting());
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::AttemptLogin(const ams::sf::InBuffer &in_path,
                                        const ams::sf::OutBuffer &out_path) {
  auto &login = getIn<IPCStruct::Login>(in_path);

  bool success = false;
  bool has2fa = false;
  std::string error;
  IPCServer::instance->executeFunction([&](NXCordClient &client) {
    success = client.setLoginCredentials(login.email, login.password, &has2fa,
                                         &error);
  });

  IPCStruct::LoginResult ret{};
  ret.success = success;
  ret.has2fa = has2fa;
  std::strncpy(ret.error_message, error.c_str(), sizeof(ret.error_message) - 1);

  setOut<IPCStruct::LoginResult>(out_path, ret);
  return ams::ResultSuccess();
}

ams::Result NXCordService::Submit2faCode(const ams::sf::InBuffer &in_path,
                                         const ams::sf::OutBuffer &out_path) {
  auto code = reinterpret_cast<const char *>(in_path.GetPointer());
  IPCServer::instance->executeFunction(
      [&code, &out_path](NXCordClient &client) {
        setOut<bool>(out_path, client.submit2faTicket(std::string(code)));
      });
  return ams::ResultSuccess();
}

ams::Result NXCordService::TokenAvailable(const ams::sf::OutBuffer &out_path) {
  IPCServer::instance->executeFunction([&out_path](NXCordClient &client) {
    setOut<bool>(out_path, client.tokenAvailable());
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::StartConnection() {
  IPCServer::instance->executeFunction(
      [](NXCordClient &client) { client.startConnection(); });
  return ams::ResultSuccess();
}

ams::Result NXCordService::StopConnection() {
  IPCServer::instance->executeFunction(
      [](NXCordClient &client) { client.quit(); });
  return ams::ResultSuccess();
}

ams::Result NXCordService::GetServers(const ams::sf::OutBuffer &out_path) {
  IPCServer::instance->executeFunction([&out_path](NXCordClient &client) {
    const std::vector<IPCStruct::DiscordServer> &cached_servers =
        client.getCachedServers();
    IPCStruct::DiscordServers servers{};
    servers.size = std::min(cached_servers.size(), sizeof(servers.servers));
    std::memcpy(servers.servers, cached_servers.data(),
                sizeof(IPCStruct::DiscordServer) * servers.size);
    setOut<IPCStruct::DiscordServers>(out_path, servers);
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::GetChannels(const ams::sf::InBuffer &in_path,
                                       const ams::sf::OutBuffer &out_path) {
  auto request = getIn<IPCStruct::DiscordChannelsRequest>(in_path);
  IPCServer::instance->executeFunction(
      [this, &request, &out_path](NXCordClient &client) {
        const std::vector<IPCStruct::DiscordChannel> &cached_channels =
            client.getCachedChannels(request.serverId);
        IPCStruct::DiscordChannels channels{};
        if (cached_channels.size() < request.offset * 100) {
          channels.size = 0;
        } else {
          channels.size = cached_channels.size() - request.offset * 100;
        }
        channels.size = std::min(channels.size, sizeof(channels.channels));
        std::memcpy(channels.channels,
                    cached_channels.data() + request.offset * 100,
                    sizeof(IPCStruct::DiscordChannel) * channels.size);
        setOut<IPCStruct::DiscordChannels>(out_path, channels);
      });
  return ams::ResultSuccess();
}

ams::Result NXCordService::JoinVoiceChannel(const ams::sf::InBuffer &in_path) {
  auto channel = getIn<IPCStruct::DiscordChannel>(in_path);
  IPCServer::instance->executeFunction([&channel](NXCordClient &client) {
    client.joinVoiceChannel(channel.serverId, channel.id);
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::DisconnectVoiceChannel() {
  IPCServer::instance->executeFunction(
      [](NXCordClient &client) { client.disconnectVoiceChannel(); });
  return ams::ResultSuccess();
}

ams::Result NXCordService::IsConnectedVoiceChannel(
    const ams::sf::OutBuffer &out_path) {
  IPCServer::instance->executeFunction([&out_path](NXCordClient &client) {
    setOut<bool>(out_path, client.isConnectedVoiceChannel());
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::Logout() {
  IPCServer::instance->executeFunction(
      [](NXCordClient &client) { client.logout(); });
  return ams::ResultSuccess();
}

ams::Result NXCordService::SetMicrophoneAmplifier(
    const ams::sf::InBuffer &in_path) {
  IPCServer::instance->executeFunction([&in_path](NXCordClient &client) {
    client.getSettings().setvoicemic_multiplier(
        std::to_string(getIn<float>(in_path)));
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::GetMicrophoneAmplifier(
    const ams::sf::OutBuffer &out_path) {
  IPCServer::instance->executeFunction([&out_path](NXCordClient &client) {
    setOut<float>(out_path,
                  std::stof(client.getSettings().getvoicemic_multiplier()));
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::SetGlobalAudioVolume(
    const ams::sf::InBuffer &in_path) {
  IPCServer::instance->executeFunction([&in_path](NXCordClient &client) {
    client.getSettings().setvoiceglobal_audio_volume(
        std::to_string(getIn<float>(in_path)));
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::GetGlobalAudioVolume(
    const ams::sf::OutBuffer &out_path) {
  IPCServer::instance->executeFunction([&out_path](NXCordClient &client) {
    setOut<float>(
        out_path,
        std::stof(client.getSettings().getvoiceglobal_audio_volume()));
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::GetMicrophoneVolume(
    const ams::sf::OutBuffer &out_path) {
  IPCServer::instance->executeFunction([&out_path](NXCordClient &client) {
    setOut<float>(out_path, client.getMicrophoneVolume());
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::SetMicrophoneThreshold(
    const ams::sf::InBuffer &in_path) {
  IPCServer::instance->executeFunction([&in_path](NXCordClient &client) {
    client.getSettings().setvoicemic_threshold(
        std::to_string(getIn<float>(in_path)));
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::GetMicrophoneThreshold(
    const ams::sf::OutBuffer &out_path) {
  IPCServer::instance->executeFunction([&out_path](NXCordClient &client) {
    setOut<float>(out_path,
                  std::stof(client.getSettings().getvoicemic_threshold()));
  });
  return ams::ResultSuccess();
}

void ipc_session_thread(IPCServer *ipc_session) {
  ipc_session->_server_manager.WaitAndProcess();
}

IPCServer::IPCServer(NXCordClient &client, std::mutex &client_mutex)
    : _client(client),
      _client_mutex(client_mutex),
      _ipc_session_thread(this, ipc_session_thread, 0x4000) {
  instance = this;
  _server_manager.RegisterServer<NXCordService>(_service_name, 1);
  _ipc_session_thread.start();
}

IPCServer::~IPCServer() {
  _ipc_session_thread.stop(
      [this]() { _server_manager.RequestStopProcessing(); });
  instance = nullptr;
}

void IPCServer::executeFunction(const Executor &executor) {
  std::scoped_lock lock(_client_mutex);
  executor(_client);
}
