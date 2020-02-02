#include "ui_logged_in.hpp"

UILoggedIn::UILoggedIn(const Interface &interface) : UICustomLayout(interface) {
  auto text = TextBlock::New(
      0, 0, "You are logged in! Press A to connect or X to logout");
  text->SetX(1280 / 2 - text->GetTextWidth() / 2);
  text->SetY(720 / 2 - text->GetTextHeight() / 2);

  if (_interface->isConnecting()) {
    onResultConnecting();
  }
  SetOnInput([this](u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
    if (Down & KEY_A) {
      _interface->startConnection();
      onResultConnecting();
    }
  });

  Add(text);
}
