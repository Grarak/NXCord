#include <cstdio>
#include <cstring>

#include "ipc_client.hpp"

IPCClient::IPCClient() {
  // TODO check for service availability
  Result rc = smGetService(&_service, SERVICE_NAME);
  printf("smGetService: 0x%x\n", rc);
}

IPCClient::~IPCClient() { serviceClose(&_service); }

bool IPCClient::isConnected() {
  bool connected;
  Result rc = serviceDispatchOut(
      &_service, static_cast<int>(CommandId::IsConnected), connected);
  printf("isConnected: %s 0x%x\n", connected ? "true" : "false", rc);
  return connected;
}

IPCStruct::LoginResult IPCClient::attemptLogin(const IPCStruct::Login& login) {
  int in = 0;
  Result rc = serviceDispatchIn(
      &_service, static_cast<int>(CommandId::AttemptLogin), in,
      .buffer_attrs = {SfBufferAttr_In | SfBufferAttr_HipcMapAlias},
      .buffers = {{&login, sizeof(login)}});
  printf("serviceDispatchIn: 0x%x\n", R_DESCRIPTION(rc));

  IPCStruct::LoginResult ret = {0};
  ret.success = true;
  return ret;
}

bool IPCClient::submit2faTicket(const std::string& code) { return false; }

bool IPCClient::tokenAvailable() { return false; }

void IPCClient::startConnection() {}

void IPCClient::stopConnection() {}

std::vector<IPCStruct::DiscordServer> IPCClient::getServers() {
  std::vector<IPCStruct::DiscordServer> servers;
  return servers;
}

std::vector<IPCStruct::DiscordChannel> IPCClient::getChannels(
    int64_t serverId) {
  std::vector<IPCStruct::DiscordChannel> channels;
  return channels;
}

void IPCClient::joinVoiceChannel(int64_t serverId, int64_t channelId) {}

void IPCClient::disconnectVoiceChannel() {}

bool IPCClient::isConnectedVoiceChannel() { return false; }

void IPCClient::logout() {}
