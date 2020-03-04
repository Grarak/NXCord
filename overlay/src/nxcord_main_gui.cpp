#include "nxcord_main_gui.hpp"

#include "nxcord_overlay_utils.hpp"
#include "nxcord_voice_channels_gui.hpp"

NXCordMainGui::NXCordMainGui(NXCordOverlay &overlay)
    : NXCordCustomGui(overlay) {}

tsl::elm::Element *NXCordMainGui::createUI() {
  auto rootFrame = defaultOverlayFrame();
  auto list = new tsl::elm::List();

  auto show_current_channel = new tsl::elm::ListItem("Show current channel");
  show_current_channel->setClickListener([this](u64 keys) {
    if (keys & KEY_A) {
      _overlay.changeTo<NXCordVoiceChannelsGui>(_overlay);
      return true;
    }
    return false;
  });

  auto leave_channel = new tsl::elm::ListItem("Leave channel");
  leave_channel->setClickListener([this](u64 keys) {
    if (keys & KEY_A) {
      _overlay.getIPCClient()->disconnectVoiceChannel();
      _overlay.clearStack();
      _overlay.changeTo<ErrorGui>(std::string(NOT_CONNECTED));
      return true;
    }
    return false;
  });

  list->addItem(show_current_channel);
  list->addItem(leave_channel);

  rootFrame->setContent(list);

  return rootFrame;
}
