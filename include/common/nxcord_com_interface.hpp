#pragma once

#include <common/ipc_structures.hpp>
#include <string>
#include <vector>

class NXCordComInterface {
 public:
  virtual ~NXCordComInterface() = default;

  virtual bool ping() = 0;

  virtual bool isConnected() = 0;
  virtual bool isConnecting() = 0;

  virtual IPCStruct::LoginResult attemptLogin(
      const IPCStruct::Login &login) = 0;
  virtual bool submit2faTicket(const std::string &code) = 0;
  virtual bool tokenAvailable() = 0;

  virtual void startConnection() = 0;
  virtual void stopConnection() = 0;

  virtual std::vector<IPCStruct::DiscordServer> getServers() = 0;
  virtual std::vector<IPCStruct::DiscordChannel> getChannels(
      int64_t serverId) = 0;

  virtual void joinVoiceChannel(int64_t serverId, int64_t channelId) = 0;
  virtual void disconnectVoiceChannel() = 0;
  virtual bool isConnectedVoiceChannel() = 0;

  virtual void logout() = 0;

  virtual void setMicrophoneAmplifier(float multiplier) = 0;
  virtual float getMicrophoneAmplifier() = 0;
  virtual void setGlobalAudioVolume(float volume) = 0;
  virtual float getGlobalAudioVolume() = 0;
  virtual float getMicrophoneVolume() = 0;
  virtual void setMicrophoneThreshold(float threshold) = 0;
  virtual float getMicrophoneThreshold() = 0;

  virtual std::vector<IPCStruct::DiscordVoiceState> getCurrentVoiceStates() = 0;
  virtual int64_t getUserID() = 0;

  virtual IPCStruct::DiscordServer getServer(int64_t serverId) = 0;
  virtual IPCStruct::DiscordChannel getConnectedVoiceChannel() = 0;

  virtual void setVoiceUserMultiplier(int64_t userId, float multiplier) = 0;
  virtual float getVoiceUserMultiplier(int64_t userId) = 0;
};
