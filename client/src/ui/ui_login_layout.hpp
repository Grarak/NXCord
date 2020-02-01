#pragma once

#include <common/utils.hpp>
#include <pu/Plutonium>

#include "elements/ui_element_edit_text.hpp"
#include "ui_custom_layout.hpp"

using pu::ui::elm::Button;
using pu::ui::elm::TextBlock;

class UILoginLayout : public UICustomLayout {
 private:
  UIElementEditText::Ref _email_edit_text;
  UIElementEditText::Ref _password_edit_text;
  Button::Ref _login_button;
  TextBlock::Ref _error_text;

  time_t _token_looked_up = 0;

 public:
  UILoginLayout(const Interface &interface);

  PU_SMART_CTOR(UILoginLayout)
};
