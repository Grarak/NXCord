#include <common/ipc_client.hpp>
#include <cstring>

IPCClient::IPCClient() {
  Result rc = smGetService(&_service, SERVICE_NAME);
  Logger::write("smGetService: 0x%x\n", rc);
}

IPCClient::~IPCClient() { serviceClose(&_service); }

bool IPCClient::submit2faTicket(const std::string &code) {
  char code_array[code.size() + 1];
  strcpy(code_array, code.c_str());
  code_array[code.size()] = '\0';
  return sendInOut<bool, char *>(CommandId::Submit2faCode, code_array);
}

std::vector<IPCStruct::DiscordServer> IPCClient::getServers() {
  std::vector<IPCStruct::DiscordServer> ret;
  auto servers = sendOut<IPCStruct::DiscordServers>(CommandId::GetServers);
  for (size_t i = 0; i < servers.size; ++i) {
    ret.push_back(servers.items[i]);
  }
  return ret;
}

std::vector<IPCStruct::DiscordChannel> IPCClient::getChannels(
    int64_t serverId) {
  std::vector<IPCStruct::DiscordChannel> ret;
  IPCStruct::DiscordChannelsRequest request = {0, serverId};

  IPCStruct::DiscordChannels channels{};
  while ((channels = sendInOut<IPCStruct::DiscordChannels,
                               IPCStruct::DiscordChannelsRequest>(
              CommandId::GetChannels, request))
             .size > 0) {
    for (size_t i = 0; i < channels.size; ++i) {
      ret.push_back(channels.items[i]);
    }
    ++request.offset;
  }
  return ret;
}

void IPCClient::joinVoiceChannel(int64_t serverId, int64_t channelId) {
  IPCStruct::DiscordChannel channel{};
  channel.serverId = serverId;
  channel.id = channelId;
  sendIn<IPCStruct::DiscordChannel>(CommandId::JoinVoiceChannel, channel);
}

std::vector<IPCStruct::DiscordVoiceState> IPCClient::getCurrentVoiceStates() {
  std::vector<IPCStruct::DiscordVoiceState> ret;
  auto states =
      sendOut<IPCStruct::DiscordVoiceStates>(CommandId::GetVoiceStates);
  for (size_t i = 0; i < states.size; ++i) {
    ret.push_back(states.items[i]);
  }
  return ret;
}
