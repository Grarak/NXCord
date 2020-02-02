#pragma once

#include <cstring>
#include <string>

#define SERVICE_NAME "nx:cord"

#define IPC_COMMAND_ENUM          \
  enum class CommandId {          \
    IsConnected = 0,              \
    AttemptLogin = 1,             \
    Submit2faCode = 2,            \
    TokenAvailable = 3,           \
    StartConnection = 4,          \
    StopConnection = 5,           \
    GetServers = 6,               \
    GetChannels = 7,              \
    JoinVoiceChannel = 8,         \
    DisconnectVoiceChannel = 9,   \
    IsConnectedVoiceChannel = 10, \
    Logout = 11,                  \
  };

namespace IPCStruct {
struct Login {
  char email[0x100];
  char password[0x100];
};

struct LoginResult {
  bool success;
  bool has2fa;
  char error_message[0x100];
};

struct DiscordServer {
  char name[0x20];
  int64_t id;
};

enum class DiscordChannelType {
  CHANNEL_TYPE_NONE = -1,
  SERVER_TEXT = 0,
  DM = 1,
  SERVER_VOICE = 2,
  GROUP_DM = 3,
  SERVER_CATEGORY = 4,
  GUILD_NEWS = 5,
  GUILD_STORE = 6
};

struct DiscordChannel {
  char name[0x20];
  int64_t serverId;
  int64_t id;
  DiscordChannelType type;
};

struct DiscordServers {
  size_t size;
  DiscordServer servers[100];  // 100 is the max number of server you can join
};

struct DiscordChannelsRequest {
  size_t offset;
  int64_t serverId;
};

struct DiscordChannels {
  size_t size;
  DiscordChannel channels[100];  // 500 is the limit of channels, pass them as
                                 // chunks, otherwise it will get too big
};
}  // namespace IPCStruct
