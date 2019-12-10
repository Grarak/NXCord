#pragma once
#include <sleepy_discord/sleepy_discord.h>

class DiscordSession : public SleepyDiscord::GenericSession {
 private:
  std::string _url;
  const std::string* _body;

  SleepyDiscord::Response request(
      const SleepyDiscord::RequestMethod method) const;

 public:
  void setUrl(const std::string& url) override;
  void setBody(const std::string* body) override;
  void setHeader(const std::vector<SleepyDiscord::HeaderPair>& header) override;
  void setMultipart(
      const std::initializer_list<SleepyDiscord::Part>& parts) override;

  SleepyDiscord::Response Post() override;
  SleepyDiscord::Response Patch() override;
  SleepyDiscord::Response Delete() override;
  SleepyDiscord::Response Get() override;
  SleepyDiscord::Response Put() override;
};
