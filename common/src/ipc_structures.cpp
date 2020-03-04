#include <common/ipc_structures.hpp>

namespace IPCStruct {
Login create_login(const std::string &email, const std::string &password) {
  IPCStruct::Login login{};
  std::strncpy(login.email, email.c_str(), sizeof(login.email) - 1);
  std::strncpy(login.password, password.c_str(), sizeof(login.password) - 1);
  return login;
}

DiscordServer create_discord_server(const std::string &name, int64_t id) {
  IPCStruct::DiscordServer discord_server{};
  std::strncpy(discord_server.name, name.c_str(),
               sizeof(discord_server.name) - 1);
  discord_server.id = id;
  return discord_server;
}

DiscordChannel create_discord_channel(const std::string &name, int64_t serverId,
                                      int64_t id,
                                      IPCStruct::DiscordChannelType type) {
  IPCStruct::DiscordChannel discord_channel{};
  std::strncpy(discord_channel.name, name.c_str(),
               sizeof(discord_channel.name) - 1);
  discord_channel.serverId = serverId;
  discord_channel.id = id;
  discord_channel.type = type;
  return discord_channel;
}

DiscordVoiceState create_discord_voice_state(int64_t userId,
                                             const std::string &name) {
  DiscordVoiceState voiceState{};
  voiceState.userId = userId;
  size_t name_size = std::min(name.size(), sizeof(voiceState.name) - 1);
  std::memcpy(voiceState.name, name.c_str(), name_size);
  voiceState.name[name_size] = '\0';
  return voiceState;
}
}  // namespace IPCStruct
