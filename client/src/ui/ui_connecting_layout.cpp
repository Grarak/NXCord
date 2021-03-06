#include "ui_connecting_layout.hpp"

#include <common/utils.hpp>

UIConnectingLayout::UIConnectingLayout(const Interface &interface)
    : UICustomLayout(interface) {
  _connecting_text =
      TextBlock::New(0, 0, "Establishing connection...Press X to stop");
  _connecting_text->SetX(SCREEN_WIDTH / 2 -
                         _connecting_text->GetTextWidth() / 2);
  _connecting_text->SetY(SCREEN_HEIGHT / 2 -
                         _connecting_text->GetTextHeight() / 2);

  if (!interface->isConnected()) {
    interface->startConnection();
  }
  AddThread([this]() {
    if (Utils::check_interval(_connection_looked_up, 1000) &&
        _interface->isConnected()) {
      onResultConnected();
    }
  });

  SetOnInput([this](u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
    if (Down & KEY_X) {
      _interface->stopConnection();
      onResultLoggedIn();
    }
  });

  Add(_connecting_text);
}
