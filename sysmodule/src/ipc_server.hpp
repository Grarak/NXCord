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

  ams::Result IsConnected(ams::sf::Out<bool> is_connected);

  ams::Result AttemptLogin(const ams::sf::InBuffer& in_path);

  ams::Result Submit2faCode(const ams::sf::InBuffer& in_path);

 public:
  DEFINE_SERVICE_DISPATCH_TABLE{
      MAKE_SERVICE_COMMAND_META(IsConnected),
      MAKE_SERVICE_COMMAND_META(AttemptLogin),
      MAKE_SERVICE_COMMAND_META(Submit2faCode),
  };
};

class IPCServer {
 public:
  using QueuedFunction = std::function<void(NXCordClient& client)>;

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

  LoopThread<IPCServer*> _ipc_session_thread;

  QueuedFunction _queued_function;
  std::mutex _queued_function_mutex;

  void queueFunction(QueuedFunction function);

  friend void ipc_session_thread(IPCServer* ipc_session);
  friend NXCordService;

 public:
  IPCServer();
  ~IPCServer();

  QueuedFunction pollQueuedFunction();
};
