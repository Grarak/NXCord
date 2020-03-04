#include "standalone_client.hpp"

#include <switch.h>

#include <common/logger.hpp>
#include <common/utils.hpp>
#include <nxcord/nxcord_settings.hpp>

void standalone_client_thread(StandaloneClient *client) {
  {
    std::scoped_lock lock(client->_nxcord_client_mutex);
    client->_nxcord_client.tick();
  }
  svcSleepThread(2e+7);
}

StandaloneClient::StandaloneClient()
    : _standalone_client_thread(this, standalone_client_thread, 0x100000) {
  Logger::write("Starting new client\n");

  auto settings = NXCordSettings::New();
  _nxcord_client.loadSettings(settings);

  _standalone_client_thread.start();
}

StandaloneClient::~StandaloneClient() { _standalone_client_thread.stop(); }

#define LOCK_CLIENT std::scoped_lock lock(_nxcord_client_mutex);

bool StandaloneClient::isConnected() {
  LOCK_CLIENT
  return _nxcord_client.isConnected();
}

bool StandaloneClient::isConnecting() {
  LOCK_CLIENT
  return _nxcord_client.isConnecting();
}

IPCStruct::LoginResult StandaloneClient::attemptLogin(
    const IPCStruct::Login &login) {
  LOCK_CLIENT
  bool has2fa = false;
  std::string error;
  bool success = _nxcord_client.setLoginCredentials(login.email, login.password,
                                                    &has2fa, &error);

  IPCStruct::LoginResult ret{};
  ret.success = success;
  ret.has2fa = has2fa;
  std::strncpy(ret.error_message, error.c_str(), sizeof(ret.error_message) - 1);
  return ret;
}

bool StandaloneClient::submit2faTicket(const std::string &code) {
  LOCK_CLIENT
  return _nxcord_client.submit2faTicket(code);
}

bool StandaloneClient::tokenAvailable() {
  LOCK_CLIENT
  return _nxcord_client.tokenAvailable();
}

void StandaloneClient::startConnection() {
  LOCK_CLIENT
  _nxcord_client.startConnection();
}

void StandaloneClient::stopConnection() {
  LOCK_CLIENT
  _nxcord_client.quit();
}

std::vector<IPCStruct::DiscordServer> StandaloneClient::getServers() {
  LOCK_CLIENT
  return _nxcord_client.getCachedServers();
}

std::vector<IPCStruct::DiscordChannel> StandaloneClient::getChannels(
    int64_t serverId) {
  LOCK_CLIENT
  return _nxcord_client.getCachedChannels(serverId);
}

void StandaloneClient::joinVoiceChannel(int64_t serverId, int64_t channelId) {
  LOCK_CLIENT
  _nxcord_client.joinVoiceChannel(serverId, channelId);
}

void StandaloneClient::disconnectVoiceChannel() {
  LOCK_CLIENT
  _nxcord_client.disconnectVoiceChannel();
}

bool StandaloneClient::isConnectedVoiceChannel() {
  LOCK_CLIENT
  return _nxcord_client.isConnectedVoiceChannel();
}

void StandaloneClient::logout() {
  LOCK_CLIENT
  _nxcord_client.logout();
}

void StandaloneClient::setMicrophoneAmplifier(float multiplier) {
  LOCK_CLIENT
  _nxcord_client.getSettings().setvoicemic_multiplier(
      std::to_string(multiplier));
}

float StandaloneClient::getMicrophoneAmplifier() {
  LOCK_CLIENT
  return std::stof(_nxcord_client.getSettings().getvoicemic_multiplier());
}

void StandaloneClient::setGlobalAudioVolume(float volume) {
  LOCK_CLIENT
  _nxcord_client.getSettings().setvoiceglobal_audio_volume(
      std::to_string(volume));
}

float StandaloneClient::getGlobalAudioVolume() {
  LOCK_CLIENT
  return std::stof(_nxcord_client.getSettings().getvoiceglobal_audio_volume());
}

float StandaloneClient::getMicrophoneVolume() {
  LOCK_CLIENT
  return _nxcord_client.getMicrophoneVolume();
}

void StandaloneClient::setMicrophoneThreshold(float threshold) {
  LOCK_CLIENT
  _nxcord_client.getSettings().setvoicemic_threshold(std::to_string(threshold));
}

float StandaloneClient::getMicrophoneThreshold() {
  LOCK_CLIENT
  return std::stof(_nxcord_client.getSettings().getvoicemic_threshold());
}

std::vector<IPCStruct::DiscordVoiceState>
StandaloneClient::getCurrentVoiceStates() {
  LOCK_CLIENT
  return _nxcord_client.getCurrentVoiceStates();
}

int64_t StandaloneClient::getUserID() {
  LOCK_CLIENT
  return _nxcord_client.getID().number();
}

IPCStruct::DiscordServer StandaloneClient::getServer(int64_t serverId) {
  LOCK_CLIENT
  return _nxcord_client.getServer(serverId);
}

IPCStruct::DiscordChannel StandaloneClient::getConnectedVoiceChannel() {
  LOCK_CLIENT
  return _nxcord_client.getConnectedVoiceChannel();
}
void StandaloneClient::setVoiceUserMultiplier(int64_t userId,
                                              float multiplier) {
  LOCK_CLIENT
  _nxcord_client.setVoiceUserMultiplier(userId, multiplier);
}

float StandaloneClient::getVoiceUserMultiplier(int64_t userId) {
  LOCK_CLIENT
  return _nxcord_client.getVoiceUserMultiplier(userId);
}
