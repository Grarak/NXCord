#include <cstdio>
#include <cstring>

#include "ipc_client.hpp"

IPCClient::IPCClient() {
  // TODO check for service availability
  Result rc = smGetService(&_service, SERVICE_NAME);
  printf("smGetService: 0x%x\n", rc);
}

IPCClient::~IPCClient() { serviceClose(&_service); }

bool IPCClient::submit2faTicket(const std::string &code) {
  char code_array[code.size() + 1];
  strcpy(code_array, code.c_str());
  code_array[code.size()] = '\0';
  return sendInOut<char *, bool>(CommandId::Submit2faCode, code_array);
}

std::vector<IPCStruct::DiscordServer> IPCClient::getServers() {
  std::vector<IPCStruct::DiscordServer> ret;
  auto servers = sendOut<IPCStruct::DiscordServers>(CommandId::GetServers);
  for (size_t i = 0; i < servers.size; ++i) {
    ret.push_back(std::move(servers.servers[i]));
  }
  return ret;
}

std::vector<IPCStruct::DiscordChannel> IPCClient::getChannels(
    int64_t serverId) {
  std::vector<IPCStruct::DiscordChannel> ret;
  IPCStruct::DiscordChannelsRequest request = {0, serverId};

  IPCStruct::DiscordChannels channels;
  while ((channels = sendInOut<IPCStruct::DiscordChannels,
                               IPCStruct::DiscordChannelsRequest>(
              CommandId::GetChannels, request))
             .size > 0) {
    for (size_t i = 0; i < channels.size; ++i) {
      ret.push_back(std::move(channels.channels[i]));
    }
    ++request.offset;
  }
  return ret;
}

void IPCClient::joinVoiceChannel(int64_t serverId, int64_t channelId) {
  IPCStruct::DiscordChannel channel;
  channel.serverId = serverId;
  channel.id = channelId;
  sendIn<IPCStruct::DiscordChannel>(CommandId::JoinVoiceChannel, channel);
}
