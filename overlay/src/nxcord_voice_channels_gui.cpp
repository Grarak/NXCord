#include "nxcord_voice_channels_gui.hpp"

#include "nxcord_user_settings.hpp"

NXCordVoiceChannelsGui::NXCordVoiceChannelsGui(NXCordOverlay& overlay)
    : NXCordCustomGui(overlay) {}

tsl::elm::Element* NXCordVoiceChannelsGui::createUI() {
  auto& ipc_client = _overlay.getIPCClient();
  IPCStruct::DiscordChannel channel = ipc_client->getConnectedVoiceChannel();
  IPCStruct::DiscordServer server = ipc_client->getServer(channel.serverId);
  int64_t user_id = ipc_client->getUserID();

  auto rootFrame = new tsl::elm::OverlayFrame(server.name, channel.name);
  auto list = new tsl::elm::List();

  std::vector<IPCStruct::DiscordVoiceState> states =
      _overlay.getIPCClient()->getCurrentVoiceStates();
  std::sort(states.begin(), states.end(),
            [](const IPCStruct::DiscordVoiceState& a,
               const IPCStruct::DiscordVoiceState& b) {
              return std::strcmp(a.name, b.name) < 0;
            });

  list->clear();
  for (const auto& state : states) {
    auto item = new tsl::elm::ListItem(state.name);
    item->setClickListener([this, user_id, state](u64 keys) {
      if (keys & KEY_A && user_id != state.userId) {
        _overlay.changeTo<NXCordUserSettings>(state, _overlay);
        return true;
      }
      return false;
    });
    list->addItem(item);
  }

  rootFrame->setContent(list);
  return rootFrame;
}
