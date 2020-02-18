#pragma once

#include "ui_custom_layout.hpp"

using pu::ui::elm::Button;
using pu::ui::elm::TextBlock;

class UIRangeSelectorLayout : public UICustomLayout {
 private:
  pu::String _title;
  std::vector<std::string> _items;
  size_t _selected;
  std::function<void(const std::string &value, size_t selected)>
      _selected_listener;

  void init();

 public:
  explicit UIRangeSelectorLayout(const Interface &interface, pu::String title,
                                 int min, int max, size_t selected = 0);

  explicit UIRangeSelectorLayout(const Interface &interface, pu::String title,
                                 std::vector<std::string> items,
                                 size_t selected = 0);

  inline void setOnSelectedListener(
      const std::function<void(const std::string &value, size_t selected)>
          &listener) {
    _selected_listener = listener;
  }

  PU_SMART_CTOR(UIRangeSelectorLayout)
};
