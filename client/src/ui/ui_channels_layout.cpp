#include "ui_channels_layout.hpp"

UIChannelsLayout::UIChannelsLayout(const Interface& interface,
                                   const IPCStruct::DiscordServer& server)
    : UICustomLayout(interface) {
  auto server_name_text = TextBlock::New(0, 0, server.name);
  int text_height = server_name_text->GetTextHeight();
  _channels_menu = Menu::New(0, text_height, 1280,
                             pu::ui::Color(0xff, 0xff, 0xff, 0xff), 100, 7);
  _channels_menu->SetOnFocusColor(pu::ui::Color(0, 0, 0, 0x80));

  AddThread([this, server]() {
    time_t current_time = Utils::current_time_millis();
    // Check for new channels each second
    if (_channels_lookup_time == 0 ||
        current_time - _channels_lookup_time >= 1000) {
      std::vector<IPCStruct::DiscordChannel> new_channels =
          _interface->getChannels(server.id);
      bool redraw = false;
      new_channels.erase(
          std::remove_if(new_channels.begin(), new_channels.end(),
                         [](const IPCStruct::DiscordChannel& channel) {
                           return channel.type !=
                                  IPCStruct::DiscordChannelType::SERVER_VOICE;
                         }),
          new_channels.end());

      if (new_channels.size() == 0) {
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
        for (const auto& channel : _channels) {
          auto item = MenuItem::New(channel.name);
          item->AddOnClick([this, &channel]() {
            if (onDialogJoinVoice()) {
              _interface->joinVoiceChannel(channel.serverId, channel.id);
            }
          });
          _channels_menu->AddItem(item);
        }
      }

      _channels_lookup_time = current_time;
    }
  });

  Add(server_name_text);
  Add(_channels_menu);
}
