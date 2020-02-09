#pragma once

#include <pu/Plutonium>

#include "elements/ui_element_edit_text.hpp"
#include "ui_custom_layout.hpp"

using pu::ui::elm::Button;
using pu::ui::elm::TextBlock;

class UILogin2faLayout : public UICustomLayout {
 private:
  UIElementEditText::Ref _code_edit_text;
  Button::Ref _submit_button;
  TextBlock::Ref _error_text;

 public:
  explicit UILogin2faLayout(const Interface &interface);

  PU_SMART_CTOR(UILogin2faLayout)
};
