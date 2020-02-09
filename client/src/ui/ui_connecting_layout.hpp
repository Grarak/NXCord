#pragma once

#include <common/utils.hpp>

#include "ui_custom_layout.hpp"

using pu::ui::elm::TextBlock;

class UIConnectingLayout : public UICustomLayout {
 private:
  TextBlock::Ref _connecting_text;
  time_t _connection_looked_up = 0;

 public:
  explicit UIConnectingLayout(const Interface &interface);

  PU_SMART_CTOR(UIConnectingLayout)
};
