#include "ui_login_2fa_layout.hpp"

UILogin2faLayout::UILogin2faLayout(const Interface &interface)
    : UICustomLayout(interface) {
  _code_edit_text =
      UIElementEditText::New(0, 0, SCREEN_HEIGHT / 2, 100, "Code");
  _submit_button = Button::New(SCREEN_WIDTH / 4 - 100, 250, 200, 100, "Submit",
                               pu::ui::Color(0xff, 0xff, 0xff, 0xff),
                               pu::ui::Color(0, 0, 0xff, 0x80));
  _error_text = TextBlock::New(0, 400, "");

  _submit_button->SetOnClick([this]() {
    _error_text->SetText("");
    std::string code = _code_edit_text->getInput();
    if (code.empty()) {
      _error_text->SetText("Code can't be empty.");
      return;
    }

    bool success = _interface->submit2faTicket(code);
    if (success) {
      onResultLoggedIn();
    } else {
      _error_text->SetText("Code is not correct.");
    }
  });

  Add(_code_edit_text);
  Add(_submit_button);
  Add(_error_text);
}
