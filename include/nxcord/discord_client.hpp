#pragma once

#include <sleepy_discord/sleepy_discord.h>

#include <mutex>

class DiscordScheduleHandler;

class DiscordClient : public SleepyDiscord::BaseDiscordClient {
 private:
  static constexpr std::string_view loginURL =
      "https://discordapp.com/api/auth/login";
  static constexpr std::string_view mfaURL =
      "https://discordapp.com/api/v6/auth/mfa/totp";

  std::string _ticket;
  std::mutex _websocket_send_mutex;

  bool connect(const std::string &, GenericMessageReceiver *,
               SleepyDiscord::WebsocketConnection &) override;

  void send(std::string, SleepyDiscord::WebsocketConnection &) override;

  void disconnect(unsigned int, std::string,
                  SleepyDiscord::WebsocketConnection &) override;

  void onError(SleepyDiscord::ErrorCode, std::string) override;

  SleepyDiscord::Timer schedule(SleepyDiscord::TimedTask code,
                                time_t milliseconds) override;

  void stopClient() override;

  friend DiscordScheduleHandler;

 protected:
  std::string _token;

 public:
  DiscordClient();

  ~DiscordClient() override;

  bool setLoginCredentials(const std::string &email,
                           const std::string &password, bool *has2fa,
                           std::string *error);

  bool submit2faTicket(const std::string &code);

  inline bool tokenAvailable() const { return !_token.empty(); }

  virtual void startConnection();

  void tick();
};
