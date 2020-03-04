#pragma once

#include <common/ipc_structures.hpp>
#include <common/loop_thread.hpp>
#include <functional>
#include <mutex>
#include <nxcord/nxcord_client.hpp>
#include <stratosphere.hpp>

class NXCordService : public ams::sf::IServiceObject {
 private:
  IPC_COMMAND_ENUM

  template <typename I>
  static inline const I &getIn(const ams::sf::InBuffer &in_path) {
    return *reinterpret_cast<const I *>(in_path.GetPointer());
  }

  template <typename O>
  static inline void setOut(const ams::sf::OutBuffer &out_path, const O &out) {
    std::memcpy(out_path.GetPointer(), &out, sizeof(out));
  }

  ams::Result Ping(const ams::sf::OutBuffer &out_path);

  ams::Result IsConnected(const ams::sf::OutBuffer &out_path);
  ams::Result IsConnecting(const ams::sf::OutBuffer &out_path);

  ams::Result AttemptLogin(const ams::sf::InBuffer &in_path,
                           const ams::sf::OutBuffer &out_path);
  ams::Result Submit2faCode(const ams::sf::InBuffer &in_path,
                            const ams::sf::OutBuffer &out_path);
  ams::Result TokenAvailable(const ams::sf::OutBuffer &out_path);

  ams::Result StartConnection();
  ams::Result StopConnection();

  ams::Result GetServers(const ams::sf::OutBuffer &out_path);
  ams::Result GetChannels(const ams::sf::InBuffer &in_path,
                          const ams::sf::OutBuffer &out_path);

  ams::Result JoinVoiceChannel(const ams::sf::InBuffer &in_path);
  ams::Result DisconnectVoiceChannel();
  ams::Result IsConnectedVoiceChannel(const ams::sf::OutBuffer &out_path);

  ams::Result Logout();

  ams::Result SetMicrophoneAmplifier(const ams::sf::InBuffer &in_path);
  ams::Result GetMicrophoneAmplifier(const ams::sf::OutBuffer &out_path);
  ams::Result SetGlobalAudioVolume(const ams::sf::InBuffer &in_path);
  ams::Result GetGlobalAudioVolume(const ams::sf::OutBuffer &out_path);
  ams::Result GetMicrophoneVolume(const ams::sf::OutBuffer &out_path);
  ams::Result SetMicrophoneThreshold(const ams::sf::InBuffer &in_path);
  ams::Result GetMicrophoneThreshold(const ams::sf::OutBuffer &out_path);

  ams::Result GetVoiceStates(const ams::sf::OutBuffer &out_path);
  ams::Result GetUserID(const ams::sf::OutBuffer &out_path);

  ams::Result GetServer(const ams::sf::InBuffer &in_path,
                        const ams::sf::OutBuffer &out_path);
  ams::Result GetConnectedVoiceChannel(const ams::sf::OutBuffer &out_path);

  ams::Result SetVoiceUserMultiplier(const ams::sf::InBuffer &in_path);
  ams::Result GetVoiceUserMultiplier(const ams::sf::InBuffer &in_path,
                                     const ams::sf::OutBuffer &out_path);

 public:
  DEFINE_SERVICE_DISPATCH_TABLE{
      MAKE_SERVICE_COMMAND_META(Ping),
      MAKE_SERVICE_COMMAND_META(IsConnected),
      MAKE_SERVICE_COMMAND_META(IsConnecting),
      MAKE_SERVICE_COMMAND_META(AttemptLogin),
      MAKE_SERVICE_COMMAND_META(Submit2faCode),
      MAKE_SERVICE_COMMAND_META(TokenAvailable),
      MAKE_SERVICE_COMMAND_META(StartConnection),
      MAKE_SERVICE_COMMAND_META(StopConnection),
      MAKE_SERVICE_COMMAND_META(GetServers),
      MAKE_SERVICE_COMMAND_META(GetChannels),
      MAKE_SERVICE_COMMAND_META(JoinVoiceChannel),
      MAKE_SERVICE_COMMAND_META(DisconnectVoiceChannel),
      MAKE_SERVICE_COMMAND_META(IsConnectedVoiceChannel),
      MAKE_SERVICE_COMMAND_META(Logout),
      MAKE_SERVICE_COMMAND_META(SetMicrophoneAmplifier),
      MAKE_SERVICE_COMMAND_META(GetMicrophoneAmplifier),
      MAKE_SERVICE_COMMAND_META(SetGlobalAudioVolume),
      MAKE_SERVICE_COMMAND_META(GetGlobalAudioVolume),
      MAKE_SERVICE_COMMAND_META(GetMicrophoneVolume),
      MAKE_SERVICE_COMMAND_META(SetMicrophoneThreshold),
      MAKE_SERVICE_COMMAND_META(GetMicrophoneThreshold),
      MAKE_SERVICE_COMMAND_META(GetVoiceStates),
      MAKE_SERVICE_COMMAND_META(GetUserID),
      MAKE_SERVICE_COMMAND_META(GetServer),
      MAKE_SERVICE_COMMAND_META(GetConnectedVoiceChannel),
      MAKE_SERVICE_COMMAND_META(SetVoiceUserMultiplier),
      MAKE_SERVICE_COMMAND_META(GetVoiceUserMultiplier),
  };
};

class IPCServer {
 public:
  using Executor = std::function<void(NXCordClient &client)>;

 private:
  static IPCServer *instance;

  struct ServerOptions {
    static const size_t PointerBufferSize = 0x1000;
    static const size_t MaxDomains = 9;
    static const size_t MaxDomainObjects = 10;
  };

  static constexpr size_t MaxServers = 4;
  static constexpr size_t MaxSessions = 40;

  ams::sm::ServiceName _service_name =
      ams::sm::ServiceName::Encode(SERVICE_NAME);
  ams::sf::hipc::ServerManager<MaxServers, ServerOptions, MaxSessions>
      _server_manager;

  NXCordClient &_client;
  std::mutex &_client_mutex;

  void executeFunction(const Executor &executor);

  LoopThread<IPCServer *> _ipc_session_thread;

  friend void ipc_session_thread(IPCServer *ipc_session);

  friend NXCordService;

 public:
  IPCServer(NXCordClient &client, std::mutex &client_mutex);

  ~IPCServer();
};
