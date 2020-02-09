#include "ui_element_edit_text.hpp"

#include <cstring>

UIElementEditText::UIElementEditText(s32 x, s32 y, s32 w, s32 h,
                                     const pu::String &title,
                                     InputType inputType)
    : _outer_rec(
          Rectangle::New(x, y, w, h, pu::ui::Color(0xff, 0xff, 0xff, 0xff), 1)),
      _inner_title_text(TextBlock::New(x, y, title)),
      _inner_input_text(TextBlock::New(x, y + h / 3, "")),
      _x(x),
      _y(y),
      _w(w),
      _h(h),
      _inputType(inputType) {}

void UIElementEditText::OnRender(pu::ui::render::Renderer::Ref &Drawer, s32 X,
                                 s32 Y) {
  _outer_rec->OnRender(Drawer, X, Y);
  _inner_title_text->OnRender(Drawer, X, Y);
  _inner_input_text->OnRender(Drawer, X, Y + _h / 3);
}

void UIElementEditText::OnInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
  _outer_rec->OnInput(Down, Up, Held, Pos);
  _inner_title_text->OnInput(Down, Up, Held, Pos);
  _inner_input_text->OnInput(Down, Up, Held, Pos);

  if (!Pos.IsEmpty() && Pos.X > _x && Pos.X < _x + _w && Pos.Y > _y &&
      Pos.Y < _y + _h) {
    SwkbdConfig kbd;
    Result rc = swkbdCreate(&kbd, 0);
    if (R_FAILED(rc)) {
      return;
    }

    char output[0x100] = {0};

    switch (_inputType) {
      case InputType::NORMAL:
        swkbdConfigMakePresetDefault(&kbd);
        break;
      case InputType::USERNAME:
        swkbdConfigMakePresetUserName(&kbd);
        break;
      case InputType::PASSWORD:
        swkbdConfigMakePresetPassword(&kbd);
        break;
    }
    swkbdConfigSetInitialText(&kbd, _input.c_str());

    rc = swkbdShow(&kbd, output, sizeof(output));

    if (R_SUCCEEDED(rc)) {
      _input = std::string(output);
      if (_inputType == InputType::PASSWORD) {
        if (strlen(output) > 0) {
          std::memset(output, '*', 6);
          output[6] = '\0';
        } else {
          output[0] = '\0';
        }
      }
      _inner_input_text->SetText(output);
    }
    swkbdClose(&kbd);
  }
}
