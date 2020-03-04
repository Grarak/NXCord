#pragma once

#include <common/utils.hpp>
#define private public
#define protected public
#include <tesla.hpp>
#undef private
#undef protected

#include "nxcord_overlay.hpp"

class NXCordCustomGui : public tsl::Gui {
 private:
  time_t _ping_lookup = 0;

 protected:
  NXCordOverlay& _overlay;

 public:
  explicit NXCordCustomGui(NXCordOverlay& overlay);

  void update() override;
};
