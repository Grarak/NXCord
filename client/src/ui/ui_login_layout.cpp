#include "ui_login_layout.hpp"

UILoginLayout::UILoginLayout(const Interface &interface)
    : UICustomLayout(interface) {
  _email_edit_text =
      UIElementEditText::New(0, 0, SCREEN_WIDTH / 2, 100, "Email");
  _password_edit_text =
      UIElementEditText::New(0, 100, SCREEN_WIDTH / 2, 100, "Password",
                             UIElementEditText::InputType::PASSWORD);
  _login_button = Button::New(SCREEN_WIDTH / 4 - 100, 250, 200, 100, "Login",
                              pu::ui::Color(0xff, 0xff, 0xff, 0xff),
                              pu::ui::Color(0, 0, 0xff, 0x80));
  _error_text = TextBlock::New(0, 400, "");

  _login_button->SetOnClick([this]() {
    _error_text->SetText("Logging in...");
    IPCStruct::LoginResult ret =
        _interface->attemptLogin(IPCStruct::create_login(
            _email_edit_text->getInput(), _password_edit_text->getInput()));
    if (ret.success) {
      onResultLoggedIn();
    } else if (ret.has2fa) {
      onResult2fa();
    } else {
      if (strlen(ret.error_message) == 0) {
        _error_text->SetText("Unknown error.");
      } else {
        _error_text->SetText(ret.error_message);
      }
    }
  });

  AddThread([this]() {
    if (Utils::check_interval(_token_looked_up, 1000) &&
        _interface->tokenAvailable()) {
      onResultLoggedIn();
    }
  });

  Add(_email_edit_text);
  Add(_password_edit_text);
  Add(_login_button);
  Add(_error_text);
}
