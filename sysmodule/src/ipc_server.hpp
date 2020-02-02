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
  inline const I& getIn(const ams::sf::InBuffer& in_path) {
    return *reinterpret_cast<const I*>(in_path.GetPointer());
  }

  template <typename O>
  inline void setOut(const ams::sf::OutBuffer& out_path, const O& out) {
    std::memcpy(out_path.GetPointer(), &out, sizeof(out));
  }

  ams::Result IsConnected(const ams::sf::OutBuffer& out_path);
  ams::Result AttemptLogin(const ams::sf::InBuffer& in_path,
                           const ams::sf::OutBuffer& out_path);
  ams::Result Submit2faCode(const ams::sf::InBuffer& in_path,
                            const ams::sf::OutBuffer& out_path);
  ams::Result TokenAvailable(const ams::sf::OutBuffer& out_path);
  ams::Result StartConnection();
  ams::Result StopConnection();
  ams::Result GetServers(const ams::sf::OutBuffer& out_path);
  ams::Result GetChannels(const ams::sf::InBuffer& in_path,
                          const ams::sf::OutBuffer& out_path);
  ams::Result JoinVoiceChannel(const ams::sf::InBuffer& in_path);
  ams::Result DisconnectVoiceChannel();
  ams::Result IsConnectedVoiceChannel(const ams::sf::OutBuffer& out_path);
  ams::Result Logout();

 public:
  DEFINE_SERVICE_DISPATCH_TABLE{
      MAKE_SERVICE_COMMAND_META(IsConnected),
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
  };
};

class IPCServer {
 public:
  using Executor = std::function<void(NXCordClient& client)>;

 private:
  static IPCServer* instance;

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

  NXCordClient& _client;
  std::mutex& _client_mutex;
  void executeFunction(Executor executor);

  LoopThread<IPCServer*> _ipc_session_thread;

  friend void ipc_session_thread(IPCServer* ipc_session);
  friend NXCordService;

 public:
  IPCServer(NXCordClient& client, std::mutex& client_mutex);
  ~IPCServer();
};
