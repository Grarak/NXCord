#pragma once

#include "nxcord_custom_gui.hpp"

class NXCordMainGui : public NXCordCustomGui {
 public:
  explicit NXCordMainGui(NXCordOverlay &overlay);

  tsl::elm::Element *createUI() override;
};
