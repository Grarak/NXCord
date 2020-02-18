#include "ui_range_selector_layout.hpp"

#include <common/utils.hpp>
#include <utility>

UIRangeSelectorLayout::UIRangeSelectorLayout(const Interface &interface,
                                             pu::String title, int min, int max,
                                             size_t selected)
    : UICustomLayout(interface), _title(std::move(title)), _selected(selected) {
  NXC_ASSERT(min < max);

  for (int i = min; i <= max; ++i) {
    _items.push_back(std::to_string(i));
  }
  init();
}

UIRangeSelectorLayout::UIRangeSelectorLayout(const Interface &interface,
                                             pu::String title,
                                             std::vector<std::string> items,
                                             size_t selected)
    : UICustomLayout(interface),
      _title(std::move(title)),
      _items(std::move(items)),
      _selected(selected) {
  init();
}

void UIRangeSelectorLayout::init() {
  auto title_text = TextBlock::New(0, 0, _title);
  title_text->SetX(SCREEN_WIDTH / 2 - title_text->GetTextWidth() / 2);

  int width_sum = 240;
  auto selected_text = TextBlock::New(0, 0, _items[_selected]);
  width_sum += selected_text->GetTextWidth();
  selected_text->SetX(SCREEN_WIDTH / 2 - selected_text->GetTextWidth() / 2);
  selected_text->SetY(SCREEN_HEIGHT / 2 - selected_text->GetTextHeight() / 2);

  auto decrease_btn = Button::New(
      SCREEN_WIDTH / 2 - width_sum / 2, SCREEN_HEIGHT / 2 - 40, 100, 80, "<",
      pu::ui::Color(0xff, 0xff, 0xff, 0xff), pu::ui::Color(0, 0, 0xff, 0xff));
  auto increase_btn = Button::New(SCREEN_WIDTH / 2 + width_sum / 2 - 100,
                                  SCREEN_HEIGHT / 2 - 40, 100, 80, ">",
                                  pu::ui::Color(0xff, 0xff, 0xff, 0xff),
                                  pu::ui::Color(0, 0, 0xff, 0xff));

  decrease_btn->SetOnClick([this, selected_text]() {
    if (_selected > 0) {
      selected_text->SetText(_items[--_selected]);
      if (_selected_listener) {
        _selected_listener(_items[_selected], _selected);
      }
    }
  });
  increase_btn->SetOnClick([this, selected_text]() {
    if (_selected + 1 < _items.size()) {
      selected_text->SetText(_items[++_selected]);
      if (_selected_listener) {
        _selected_listener(_items[_selected], _selected);
      }
    }
  });

  Add(title_text);
  Add(decrease_btn);
  Add(selected_text);
  Add(increase_btn);
}
