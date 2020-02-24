#pragma once

#include "ui_custom_layout.hpp"

using pu::ui::elm::TextBlock;

class UINoSysmodule : public UICustomLayout {
 public:
  explicit UINoSysmodule(const Interface &interface, bool crashed);

  PU_SMART_CTOR(UINoSysmodule)
};
