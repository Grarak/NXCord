#pragma once

#include <pu/Plutonium>

#include "ui_custom_layout.hpp"

using pu::ui::elm::TextBlock;

class UILoggedIn : public UICustomLayout {
 public:
  UILoggedIn(const Interface &interface);

  PU_SMART_CTOR(UILoggedIn)
};
