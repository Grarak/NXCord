#include "ui_channels_layout.hpp"

UIChannelsLayout::UIChannelsLayout(const Interface &interface,
                                   const IPCStruct::DiscordServer &server)
    : UICustomLayout(interface) {
  auto server_name_text =
      TextBlock::New(0, 0, server.name + std::string(" - Voice channels"));
  server_name_text->SetX(SCREEN_WIDTH / 2 -
                         server_name_text->GetTextWidth() / 2);
  _channels_menu = Menu::New(0, server_name_text->GetTextHeight(), SCREEN_WIDTH,
                             pu::ui::Color(0xff, 0xff, 0xff, 0xff), 50, 14);
  _channels_menu->SetOnFocusColor(pu::ui::Color(0, 0, 0, 0x80));

  AddThread([this, server]() {
    if (Utils::check_interval(_channels_lookup_time, 1000)) {
      std::vector<IPCStruct::DiscordChannel> new_channels =
          _interface->getChannels(server.id);
      bool redraw = false;
      new_channels.erase(
          std::remove_if(new_channels.begin(), new_channels.end(),
                         [](const IPCStruct::DiscordChannel &channel) {
                           return channel.type !=
                                  IPCStruct::DiscordChannelType::SERVER_VOICE;
                         }),
          new_channels.end());

      if (new_channels.empty()) {
        onResultNoChannels();
        return;
      }

      if (new_channels.size() != _channels.size()) {
        _channels = std::move(new_channels);
        redraw = true;
      } else {
        for (size_t i = 0; i < new_channels.size(); ++i) {
          if (new_channels[i].id != _channels[i].id) {
            _channels = std::move(new_channels);
            redraw = true;
            break;
          }
        }
      }

      if (redraw) {
        _channels_menu->ClearItems();
        for (const auto &channel : _channels) {
          auto item = MenuItem::New(channel.name);
          item->AddOnClick([this, &channel]() {
            if (onDialogJoinVoice()) {
              _interface->joinVoiceChannel(channel.serverId, channel.id);
            }
          });
          _channels_menu->AddItem(item);
        }
      }
    }
  });

  Add(server_name_text);
  Add(_channels_menu);
}
