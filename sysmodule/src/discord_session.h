#pragma once
#include <sleepy_discord/sleepy_discord.h>

#include "mbedtls_wrapper.h"

class DiscordSession : public SleepyDiscord::GenericSession {
 private:
  std::string _url;
  const std::string* _body = nullptr;
  const std::vector<SleepyDiscord::HeaderPair>* _headers = nullptr;

  bool contains_header(const std::string &key) const;

 public:
  void setUrl(const std::string& url) override;
  void setBody(const std::string* body) override;
  void setHeader(const std::vector<SleepyDiscord::HeaderPair>& header) override;
  void setMultipart(
      const std::initializer_list<SleepyDiscord::Part>& parts) override;

  std::shared_ptr<MBedTLSWrapper> request(
      const SleepyDiscord::RequestMethod method,
      SleepyDiscord::Response* response);

  SleepyDiscord::Response Post() override;
  SleepyDiscord::Response Patch() override;
  SleepyDiscord::Response Delete() override;
  SleepyDiscord::Response Get() override;
  SleepyDiscord::Response Put() override;
};
