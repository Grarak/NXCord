#include "nxcord_custom_gui.hpp"

#include "error_gui.hpp"
#include "nxcord_overlay_utils.hpp"

NXCordCustomGui::NXCordCustomGui(NXCordOverlay& overlay) : _overlay(overlay) {}

void NXCordCustomGui::update() {
  if (Utils::check_interval(_ping_lookup, 1000)) {
    if (_overlay.getIPCClient()->ping()) {
      if (!_overlay.getIPCClient()->isConnectedVoiceChannel()) {
        _overlay.clearStack();
        _overlay.changeTo<ErrorGui>(std::string(NOT_CONNECTED));
      }
    } else {
      _overlay.clearStack();
      _overlay.changeTo<ErrorGui>(std::string(NO_SYSMODULE));
    }
  }
}
