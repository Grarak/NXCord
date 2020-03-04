#pragma once

#include "nxcord_custom_gui.hpp"

class NXCordVoiceChannelsGui : public NXCordCustomGui {
 private:
  time_t _states_lookup = 0;

 public:
  explicit NXCordVoiceChannelsGui(NXCordOverlay& overlay);

  tsl::elm::Element* createUI() override;
};
