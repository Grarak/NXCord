#pragma once

#include "nxcord_overlay_utils.hpp"

class ErrorGui : public tsl::Gui {
 private:
  std::string _error;

 public:
  explicit ErrorGui(std::string error);

  tsl::elm::Element* createUI() override;
};
