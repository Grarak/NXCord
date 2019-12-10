#include <curl/curl.h>

#include "discord_client.h"

DiscordClient::DiscordClient(const std::string& token) : _token(token) {
  curl_global_init(CURL_GLOBAL_ALL);
  start(_token);
}

DiscordClient::~DiscordClient() { curl_global_cleanup(); }

bool DiscordClient::connect(const std::string& url) {
  return false;
}
