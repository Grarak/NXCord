#include "ui_no_sysmodule.hpp"

UINoSysmodule::UINoSysmodule(const UICustomLayout::Interface& interface,
                             bool crashed)
    : UICustomLayout(interface) {
  auto text = TextBlock::New(
      0, 0,
      crashed
          ? "NXCord sysmodule crashed, make sure your account is not too big."
          : "Couldn't communicate with sysmodule, either it's not installed or "
            "has crashed!");
  text->SetX(SCREEN_WIDTH / 2 - text->GetTextWidth() / 2);
  text->SetY(SCREEN_HEIGHT / 2 - text->GetTextHeight() / 2);

  Add(text);
}
