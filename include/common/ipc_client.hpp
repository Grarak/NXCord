#pragma once

#include <switch.h>

#include <string>

#include "ipc_structures.hpp"
#include "logger.hpp"
#include "nxcord_com_interface.hpp"

class IPCClient : public NXCordComInterface {
 private:
  IPC_COMMAND_ENUM

  Service _service{};

  void send(CommandId command) {
    Result rc = serviceDispatch(&_service, static_cast<int>(command));
    if (R_FAILED(rc)) {
      Logger::write("Failed to send %d 0x%x\n", command, rc);
    } else {
      Logger::write("Successfully send %d\n", command);
    }
  }

  template <typename R>
  R sendOut(CommandId command) {
    int placeholder_id = 0;
    R ret{};
    Result rc = serviceDispatchOut(
        &_service, static_cast<int>(command), placeholder_id,
        .buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
        .buffers = {
            {&ret, sizeof(ret)},
        });
    if (R_FAILED(rc)) {
      Logger::write("Failed to send %d 0x%x\n", command, rc);
    } else {
      Logger::write("Successfully send %d\n", command);
    }
    return ret;
  }

  template <typename R, typename A>
  R sendInOut(CommandId command, const A &in_arg) {
    int placeholder_in_id = 0;
    int placeholder_out_id = 0;
    R ret{};
    Result rc = serviceDispatchInOut(
        &_service, static_cast<int>(command), placeholder_in_id,
        placeholder_out_id,
        .buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In,
                         SfBufferAttr_HipcMapAlias | SfBufferAttr_Out},
        .buffers = {{&in_arg, sizeof(in_arg)}, {&ret, sizeof(ret)}});
    if (R_FAILED(rc)) {
      Logger::write("Failed to send %d\n", command);
    } else {
      Logger::write("Successfully send %d\n", command);
    }
    return ret;
  }

  template <typename A>
  void sendIn(CommandId command, const A &in_arg) {
    int placeholder_id = 0;
    Result rc = serviceDispatchIn(
        &_service, static_cast<int>(command), placeholder_id,
        .buffer_attrs = {SfBufferAttr_HipcMapAlias | SfBufferAttr_In},
        .buffers = {{&in_arg, sizeof(in_arg)}});
    if (R_FAILED(rc)) {
      Logger::write("Failed to send %d\n", command);
    } else {
      Logger::write("Successfully send %d\n", command);
    }
  }

 public:
  IPCClient();

  ~IPCClient() override;

  inline bool ping() override { return sendOut<bool>(CommandId::Ping); }

  inline bool isConnected() override {
    return sendOut<bool>(CommandId::IsConnected);
  }

  inline bool isConnecting() override {
    return sendOut<bool>(CommandId::IsConnecting);
  }

  inline IPCStruct::LoginResult attemptLogin(
      const IPCStruct::Login &login) override {
    return sendInOut<IPCStruct::LoginResult, IPCStruct::Login>(
        CommandId::AttemptLogin, login);
  }

  bool submit2faTicket(const std::string &code) override;

  inline bool tokenAvailable() override {
    return sendOut<bool>(CommandId::TokenAvailable);
  }

  inline void startConnection() override { send(CommandId::StartConnection); }

  inline void stopConnection() override { send(CommandId::StopConnection); }

  std::vector<IPCStruct::DiscordServer> getServers() override;

  std::vector<IPCStruct::DiscordChannel> getChannels(int64_t serverId) override;

  void joinVoiceChannel(int64_t serverId, int64_t channelId) override;

  inline void disconnectVoiceChannel() override {
    send(CommandId::DisconnectVoiceChannel);
  }

  inline bool isConnectedVoiceChannel() override {
    return sendOut<bool>(CommandId::IsConnectedVoiceChannel);
  }

  inline void logout() override { send(CommandId::Logout); }

  inline void setMicrophoneAmplifier(float multiplier) override {
    sendIn<float>(CommandId::SetMicrophoneAmplifier, multiplier);
  }

  inline float getMicrophoneAmplifier() override {
    return sendOut<float>(CommandId::GetMicrophoneAmplifier);
  }

  inline void setGlobalAudioVolume(float volume) override {
    sendIn<float>(CommandId::SetGlobalAudioVolume, volume);
  }

  inline float getGlobalAudioVolume() override {
    return sendOut<float>(CommandId::GetGlobalAudioVolume);
  }

  inline float getMicrophoneVolume() override {
    return sendOut<float>(CommandId::GetMicrophoneVolume);
  }

  inline void setMicrophoneThreshold(float threshold) override {
    sendIn<float>(CommandId::SetMicrophoneThreshold, threshold);
  }

  inline float getMicrophoneThreshold() override {
    return sendOut<float>(CommandId::GetMicrophoneThreshold);
  }

  std::vector<IPCStruct::DiscordVoiceState> getCurrentVoiceStates() override;

  inline int64_t getUserID() override {
    return sendOut<int64_t>(CommandId::GetUserID);
  }

  inline IPCStruct::DiscordServer getServer(int64_t serverId) override {
    return sendInOut<IPCStruct::DiscordServer, int64_t>(CommandId::GetServer,
                                                        serverId);
  }

  inline IPCStruct::DiscordChannel getConnectedVoiceChannel() override {
    return sendOut<IPCStruct::DiscordChannel>(
        CommandId::GetConnectedVoiceChannel);
  }

  inline void setVoiceUserMultiplier(int64_t userId,
                                     float multiplier) override {
    sendIn<IPCStruct::DiscordVoiceUserMultiplier>(
        CommandId::SetVoiceUserMultiplier, {userId, multiplier});
  }

  inline float getVoiceUserMultiplier(int64_t userId) override {
    return sendInOut<float, int64_t>(CommandId::GetVoiceUserMultiplier, userId);
  }
};
