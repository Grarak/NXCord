#pragma once

#define private public
#define protected public
#include <tesla.hpp>
#undef private
#undef protected

#include <common/ipc_client.hpp>

#include "error_gui.hpp"

class NXCordOverlay : public tsl::Overlay {
 private:
  std::unique_ptr<IPCClient> _ipc_client;

 public:
  void initServices() override;

  void exitServices() override;

  std::unique_ptr<tsl::Gui> loadInitialGui() override;

  inline void clearStack() {
    while (!m_guiStack.empty()) {
      m_guiStack.pop();
    }
  }

  inline const std::unique_ptr<IPCClient>& getIPCClient() {
    return _ipc_client;
  }
};
