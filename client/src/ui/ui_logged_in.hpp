#pragma once

#include <common/utils.hpp>
#include <pu/Plutonium>

#include "ui_custom_layout.hpp"

using pu::ui::elm::TextBlock;

class UILoggedIn : public UICustomLayout {
 private:
  time_t _connection_looked_up = 0;

 public:
  explicit UILoggedIn(const Interface &interface);

  PU_SMART_CTOR(UILoggedIn)
};
