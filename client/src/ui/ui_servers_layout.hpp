#pragma once

#include <common/utils.hpp>

#include "ui_custom_layout.hpp"

using pu::ui::elm::Menu;
using pu::ui::elm::MenuItem;

class UIServersLayout : public UICustomLayout {
 private:
  Menu::Ref _server_menu;
  std::vector<IPCStruct::DiscordServer> _servers;
  time_t _server_lookup_time = 0;

 public:
  UIServersLayout(const Interface& interface);

  PU_SMART_CTOR(UIServersLayout)
};
