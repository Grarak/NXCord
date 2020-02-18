#include "ui_microphone_sensitivity_layout.hpp"

UIMicrophoneSensitivityLayout::UIMicrophoneSensitivityLayout(
    const UICustomLayout::Interface& interface)
    : UICustomLayout(interface) {
  if (!interface->isConnectedVoiceChannel()) {
    auto error = TextBlock::New(
        0, 0,
        "This setting is only available when connected to a voice channel");
    error->SetX(SCREEN_WIDTH / 2 - error->GetTextWidth() / 2);
    error->SetY(SCREEN_HEIGHT / 2 - error->GetTextHeight() / 2);
    Add(error);
    return;
  }

  auto title = TextBlock::New(0, 0, "Microphone sensitivity");
  title->SetX(SCREEN_WIDTH / 2 - title->GetTextWidth() / 2);

  int sensitivity_width = 900;
  int sensitivity_height = 80;
  int sensitivity_x = SCREEN_WIDTH / 2 - sensitivity_width / 2;
  int sensitivity_y = SCREEN_HEIGHT / 2 - sensitivity_height / 2;
  size_t threshold_steps = 100;
  size_t threshold_step = sensitivity_width / threshold_steps;

  auto sensitivity_bg =
      Rectangle::New(sensitivity_x, sensitivity_y, sensitivity_width,
                     sensitivity_height, pu::ui::Color(0x00, 0x00, 0x00, 0x50));
  auto sensitivity =
      Rectangle::New(sensitivity_x, sensitivity_y, 0, sensitivity_height,
                     pu::ui::Color(0x00, 0xff, 0x00, 0xff));

  _current_threshold = _interface->getMicrophoneThreshold() * threshold_steps;
  auto threshold = Rectangle::New(
      sensitivity_x + _current_threshold * threshold_step, sensitivity_y - 10,
      5, sensitivity_height + 20, pu::ui::Color(0xff, 0x00, 0x00, 0xff));

  auto decrease_btn = Button::New(sensitivity_x - 120, sensitivity_y, 100, 80,
                                  "<", pu::ui::Color(0xff, 0xff, 0xff, 0xff),
                                  pu::ui::Color(0x00, 0x00, 0xff, 0xff));
  auto increase_btn =
      Button::New(sensitivity_x + sensitivity_width + 20, sensitivity_y, 100,
                  80, ">", pu::ui::Color(0xff, 0xff, 0xff, 0xff),
                  pu::ui::Color(0x00, 0x00, 0xff, 0xff));

  auto decrease_fun = [this, sensitivity_x, threshold, threshold_steps,
                       threshold_step]() {
    if (_current_threshold > 0) {
      threshold->SetX(sensitivity_x + --_current_threshold * threshold_step);
      _interface->setMicrophoneThreshold(
          static_cast<float>(_current_threshold) / threshold_steps);
    }
  };

  auto increase_fun = [this, sensitivity_x, threshold, threshold_steps,
                       threshold_step]() {
    if (_current_threshold < threshold_steps) {
      threshold->SetX(sensitivity_x + ++_current_threshold * threshold_step);
      _interface->setMicrophoneThreshold(
          static_cast<float>(_current_threshold) / threshold_steps);
    }
  };

  decrease_btn->SetOnClick(decrease_fun);
  increase_btn->SetOnClick(increase_fun);

  SetOnInput([decrease_fun, increase_fun](u64 Down, u64 Up, u64 Held,
                                          pu::ui::Touch Pos) {
    if (Down & KEY_DLEFT || Down & KEY_LSTICK_LEFT || Down & KEY_RSTICK_LEFT) {
      decrease_fun();
    } else if (Down & KEY_DRIGHT || Down & KEY_LSTICK_RIGHT ||
               Down & KEY_RSTICK_RIGHT) {
      increase_fun();
    }
  });

  AddThread([this, sensitivity, sensitivity_width]() {
    if (Utils::check_interval(_sensitivity_looked_up, 100)) {
      float volume = _interface->getMicrophoneVolume();
      sensitivity->SetWidth(volume * static_cast<float>(sensitivity_width));
    }
  });

  Add(title);
  Add(decrease_btn);
  Add(sensitivity_bg);
  Add(sensitivity);
  Add(threshold);
  Add(increase_btn);
}
