#pragma once

#include <switch.h>

#include <pu/Plutonium>

using pu::ui::elm::Rectangle;
using pu::ui::elm::TextBlock;

class UIElementEditText : public pu::ui::elm::Element {
 public:
  enum class InputType {
    NORMAL,
    USERNAME,
    PASSWORD,
  };

  UIElementEditText(s32 x, s32 y, s32 w, s32 h, const pu::String &title,
                    InputType inputType = InputType::NORMAL);

  PU_SMART_CTOR(UIElementEditText)

  inline s32 GetX() override { return _x; }

  inline s32 GetY() override { return _y; }

  inline s32 GetWidth() override { return _w; }

  inline s32 GetHeight() override { return _h; }

  void OnRender(pu::ui::render::Renderer::Ref &Drawer, s32 X, s32 Y) override;

  void OnInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) override;

  [[nodiscard]] inline std::string getInput() const { return _input; }

 private:
  Rectangle::Ref _outer_rec;
  TextBlock::Ref _inner_title_text;
  TextBlock::Ref _inner_input_text;

  s32 _x;
  s32 _y;
  s32 _w;
  s32 _h;
  InputType _inputType;
  std::string _input;
};
