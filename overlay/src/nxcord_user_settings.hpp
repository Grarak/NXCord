#pragma once

#include "nxcord_custom_gui.hpp"

class NXCordUserSettings : public NXCordCustomGui {
 private:
  IPCStruct::DiscordVoiceState _voice_state;
  float _current_voice_multiplier = 1;

 public:
  explicit NXCordUserSettings(const IPCStruct::DiscordVoiceState& state,
                              NXCordOverlay& overlay);

  tsl::elm::Element* createUI() override;
};
