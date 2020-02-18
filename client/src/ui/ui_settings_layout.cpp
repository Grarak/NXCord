#include "ui_settings_layout.hpp"

#include "ui_microphone_sensitivity_layout.hpp"
#include "ui_range_selector_layout.hpp"

UISettingsLayout::UISettingsLayout(const UICustomLayout::Interface& interface)
    : UICustomLayout(interface) {
  auto text = TextBlock::New(0, 0, "Settings");
  text->SetX(SCREEN_WIDTH / 2 - text->GetTextWidth() / 2);

  auto menu = Menu::New(0, text->GetTextHeight(), SCREEN_WIDTH,
                        pu::ui::Color(0xff, 0xff, 0xff, 0xff), 100, 10);
  menu->SetOnFocusColor(pu::ui::Color(0, 0, 0, 0x80));

  auto microphone_amplifier = MenuItem::New("Microphone amplifier");
  microphone_amplifier->AddOnClick([this]() {
    std::vector<std::string> items;
    for (int i = 1; i <= 49; ++i) {
      items.push_back(
          std::to_string(0.2f * static_cast<float>(i)).substr(0, 3) + "x");
    }

    auto selector = UIRangeSelectorLayout::New(
        _interface, "Microphone amplifier", items,
        _interface->getMicrophoneAmplifier() / 0.2f - 1);
    selector->setOnSelectedListener(
        [this](const std::string& value, size_t selected) {
          _interface->setMicrophoneAmplifier((selected + 1.0f) * 0.2f);
        });
    onShowLayout(selector);
  });
  menu->AddItem(microphone_amplifier);

  auto global_audio_volume = MenuItem::New("Global audio volume");
  global_audio_volume->AddOnClick([this]() {
    std::vector<std::string> items;
    for (int i = 1; i <= 20; ++i) {
      items.push_back(
          std::to_string(0.1f * static_cast<float>(i)).substr(0, 3) + "x");
    }

    auto selector = UIRangeSelectorLayout::New(
        _interface, "Global audio volume", items,
        _interface->getGlobalAudioVolume() / 0.1f - 1);
    selector->setOnSelectedListener(
        [this](const std::string& value, size_t selected) {
          _interface->setGlobalAudioVolume((selected + 1.0f) * 0.1f);
        });
    onShowLayout(selector);
  });
  menu->AddItem(global_audio_volume);

  auto microphone_sensitivity = MenuItem::New("Microphone sensitivity");
  microphone_sensitivity->AddOnClick([this]() {
    auto microphone_sensitivity_layout =
        UIMicrophoneSensitivityLayout::New(_interface);
    onShowLayout(microphone_sensitivity_layout);
  });
  menu->AddItem(microphone_sensitivity);

  Add(text);
  Add(menu);
}
