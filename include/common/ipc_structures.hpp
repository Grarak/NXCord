#pragma once

#include <cstring>
#include <string>

#define SERVICE_NAME "nx:cord"

#define IPC_COMMAND_ENUM \
  enum class CommandId { \
    IsConnected = 0,     \
    AttemptLogin = 1,    \
    Submit2faCode = 2,   \
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
}  // namespace IPCStruct
