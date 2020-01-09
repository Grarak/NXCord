#include <common/logger.hpp>

#include "ipc_server.hpp"

IPCServer* IPCServer::instance = nullptr;

ams::Result NXCordService::IsConnected(ams::sf::Out<bool> is_connected) {
  IPCServer::instance->queueFunction([&is_connected](NXCordClient& client) {
    is_connected.SetValue(client.isConnected());
  });
  return ams::ResultSuccess();
}

ams::Result NXCordService::AttemptLogin(const ams::sf::InBuffer& in_path) {
  auto login = reinterpret_cast<const IPCStruct::Login*>(in_path.GetPointer());
  Logger::write("Login %s %s\n", login->email, login->password);

  // IPCStruct::LoginResult result = {true, true};
  // std::memcpy(result.error_message, "hello", 6);
  // std::memcpy(out_path.GetPointer(), "test2", 5);
  /*IPCServer::instance->queueFunction(
      [login](NXCordClient& client) { client.quit();
          client.setLoginCredentials(login->);
      });*/
  return ams::ResultSuccess();
}

ams::Result NXCordService::Submit2faCode(const ams::sf::InBuffer& in_path) {
  return ams::ResultSuccess();
}

void ipc_session_thread(IPCServer* ipc_session) {
  ipc_session->_server_manager.WaitAndProcess();
}

IPCServer::IPCServer() : _ipc_session_thread(this, ipc_session_thread, 0x4000) {
  instance = this;
  _server_manager.RegisterServer<NXCordService>(_service_name, 1);
  _ipc_session_thread.start();
}

IPCServer::~IPCServer() {
  _ipc_session_thread.stop(
      [this]() { _server_manager.RequestStopProcessing(); });
}

void IPCServer::queueFunction(QueuedFunction function) {
  Barrier sync_barrier;  // Barrier works like a countdown
  barrierInit(&sync_barrier, 2);

  _queued_function_mutex.lock();
  _queued_function = [&sync_barrier, function](NXCordClient& client) {
    function(client);
    barrierWait(&sync_barrier);
  };
  _queued_function_mutex.unlock();

  barrierWait(&sync_barrier);
}

IPCServer::QueuedFunction IPCServer::pollQueuedFunction() {
  std::scoped_lock lock(_queued_function_mutex);
  QueuedFunction func = std::move(_queued_function);
  _queued_function = QueuedFunction();
  return func;
}
