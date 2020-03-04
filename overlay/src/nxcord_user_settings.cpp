#include "nxcord_user_settings.hpp"

NXCordUserSettings::NXCordUserSettings(
    const IPCStruct::DiscordVoiceState& state, NXCordOverlay& overlay)
    : NXCordCustomGui(overlay), _voice_state(state) {}

tsl::elm::Element* NXCordUserSettings::createUI() {
  auto rootFrame =
      new tsl::elm::OverlayFrame(_voice_state.name, "Voice settings");
  auto list = new tsl::elm::List();

  _current_voice_multiplier =
      _overlay.getIPCClient()->getVoiceUserMultiplier(_voice_state.userId);
  auto increase = new tsl::elm::ListItem("Increase volume");
  increase->setClickListener([this](u64 keys) {
    if (keys & KEY_A) {
      _current_voice_multiplier =
          std::min(2.0f, _current_voice_multiplier + 0.1f);
      _overlay.getIPCClient()->setVoiceUserMultiplier(
          _voice_state.userId, _current_voice_multiplier);
      return true;
    }
    return false;
  });
  auto decrease = new tsl::elm::ListItem("Decrease volume");
  decrease->setClickListener([this](u64 keys) {
    if (keys & KEY_A) {
      _current_voice_multiplier =
          std::max(.0f, _current_voice_multiplier - 0.1f);
      _overlay.getIPCClient()->setVoiceUserMultiplier(
          _voice_state.userId, _current_voice_multiplier);
      return true;
    }
    return false;
  });

  list->addItem(increase);
  list->addItem(decrease);

  rootFrame->setContent(list);
  return rootFrame;
}
