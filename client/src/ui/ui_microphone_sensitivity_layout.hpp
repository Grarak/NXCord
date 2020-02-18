#pragma once

#include <common/utils.hpp>

#include "ui_custom_layout.hpp"

using pu::ui::elm::Button;
using pu::ui::elm::Rectangle;
using pu::ui::elm::TextBlock;

class UIMicrophoneSensitivityLayout : public UICustomLayout {
 private:
  time_t _sensitivity_looked_up = 0;
  size_t _current_threshold = 0;

 public:
  explicit UIMicrophoneSensitivityLayout(const Interface &interface);

  PU_SMART_CTOR(UIMicrophoneSensitivityLayout)
};
