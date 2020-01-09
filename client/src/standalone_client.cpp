#ifdef STANDALONE
#include <switch.h>

#include <common/logger.hpp>
#include <common/utils.hpp>
#include <nxcord/nxcord_settings.hpp>

#include "standalone_client.hpp"

void standalone_client_thread(StandaloneClient *client) {
  Logger::write("Starting new client\n");

  NXCordClient nxcord_client;
  auto settings = NXCordSettings::New();
  nxcord_client.loadSettings(settings);

  client->_nxcord_client = &nxcord_client;
  while (client->_standalone_client_thread.isActive()) {
    client->_nxcord_client_mutex.lock();
    nxcord_client.tick();
    client->_nxcord_client_mutex.unlock();
    svcSleepThread(2e+7);
  }
  client->_nxcord_client = nullptr;
}

StandaloneClient::StandaloneClient()
    : _standalone_client_thread(this, standalone_client_thread, 0x200000,
                                false) {
  _standalone_client_thread.start();
}

StandaloneClient::~StandaloneClient() { _standalone_client_thread.stop(); }

#define LOCK_CLIENT std::scoped_lock lock(_nxcord_client_mutex);

bool StandaloneClient::isConnected() {
  LOCK_CLIENT
  return _nxcord_client->isConnected();
}

IPCStruct::LoginResult StandaloneClient::attemptLogin(
    const IPCStruct::Login &login) {
  LOCK_CLIENT
  bool has2fa = false;
  std::string error;
  bool success = _nxcord_client->setLoginCredentials(
      login.email, login.password, &has2fa, &error);

  IPCStruct::LoginResult ret;
  ret.success = success;
  ret.has2fa = has2fa;
  std::strncpy(ret.error_message, error.c_str(), sizeof(ret.error_message) - 1);
  return ret;
}

bool StandaloneClient::submit2faTicket(const std::string &code) {
  LOCK_CLIENT
  return _nxcord_client->submit2faTicket(code);
}

bool StandaloneClient::tokenAvailable() {
  LOCK_CLIENT
  return _nxcord_client->tokenAvailable();
}

void StandaloneClient::startConnection() {
  LOCK_CLIENT
  _nxcord_client->startConnection();
}

void StandaloneClient::stopConnection() {
  LOCK_CLIENT
  _nxcord_client->quit();
}

std::vector<IPCStruct::DiscordServer> StandaloneClient::getServers() {
  LOCK_CLIENT
  return _nxcord_client->getCachedServers();
}

std::vector<IPCStruct::DiscordChannel> StandaloneClient::getChannels(
    int64_t serverId) {
  LOCK_CLIENT
  return _nxcord_client->getCachedChannels(serverId);
}

void StandaloneClient::joinVoiceChannel(int64_t serverId, int64_t channelId) {
  LOCK_CLIENT
  nxcord_client->joinVoiceChannel(serverId, channelId);
}

void StandaloneClient::disconnectVoiceChannel() {
  LOCK_CLIENT
  _nxcord_client->disconnectVoiceChannel();
}

bool StandaloneClient::isConnectedVoiceChannel() {
  LOCK_CLIENT
  return _nxcord_client->isConnectedVoiceChannel();
}

void StandaloneClient::logout() {
  LOCK_CLIENT
  _nxcord_client->logout();
}

#endif
