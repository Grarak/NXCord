#pragma once

#include <pu/Plutonium>

#include "ui_custom_layout.hpp"

using pu::ui::elm::Menu;
using pu::ui::elm::MenuItem;

class UISettingsLayout : public UICustomLayout {
 public:
  explicit UISettingsLayout(const Interface &interface);

  PU_SMART_CTOR(UISettingsLayout)
};
