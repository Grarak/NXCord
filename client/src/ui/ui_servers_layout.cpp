#include "ui_servers_layout.hpp"

UIServersLayout::UIServersLayout(const Interface& interface)
    : UICustomLayout(interface) {
  _server_menu =
      Menu::New(0, 0, 1280, pu::ui::Color(0xff, 0xff, 0xff, 0xff), 100, 8);
  _server_menu->SetOnFocusColor(pu::ui::Color(0, 0, 0, 0x80));

  AddThread([this]() {
    if (Utils::check_interval(_server_lookup_time, 1000)) {
      std::vector<IPCStruct::DiscordServer> new_servers =
          _interface->getServers();
      bool redraw = false;
      if (new_servers.size() != _servers.size()) {
        _servers = std::move(new_servers);
        redraw = true;
      } else {
        for (size_t i = 0; i < new_servers.size(); ++i) {
          if (new_servers[i].id != _servers[i].id) {
            _servers = std::move(new_servers);
            redraw = true;
            break;
          }
        }
      }

      if (redraw) {
        _server_menu->ClearItems();
        for (const auto& server : _servers) {
          auto item = MenuItem::New(server.name);
          item->AddOnClick([this, &server]() { onResultServerClick(server); });
          _server_menu->AddItem(item);
        }
      }
    }
  });

  Add(_server_menu);
}
