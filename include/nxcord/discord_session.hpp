#pragma once
#include <sleepy_discord/sleepy_discord.h>

#include "mbedtls_wrapper.hpp"

class DiscordSession : public SleepyDiscord::GenericSession {
 private:
  std::string _url;
  const std::string* _body = nullptr;
  const std::vector<SleepyDiscord::HeaderPair>* _headers = nullptr;

  bool contains_header(const std::string& key) const;

 public:
  void setUrl(const std::string& url) override;
  void setBody(const std::string* body) override;
  void setHeader(const std::vector<SleepyDiscord::HeaderPair>& header) override;
  void setMultipart(
      const std::initializer_list<SleepyDiscord::Part>& parts) override {
  }  // TODO: Implement
  void setResponseCallback(const ResponseCallback& callback) override {
  }  // Not used anywhere
  SleepyDiscord::Response request(SleepyDiscord::RequestMethod method) override;

  std::unique_ptr<MBedTLSWrapper> request(
      const SleepyDiscord::RequestMethod method,
      SleepyDiscord::Response* response);
};
