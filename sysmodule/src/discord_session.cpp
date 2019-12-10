#include <curl/curl.h>

#include <map>

#include "discord_session.h"

SleepyDiscord::CustomInit SleepyDiscord::Session::init =
    []() -> SleepyDiscord::GenericSession* { return new DiscordSession; };

void DiscordSession::setUrl(const std::string& url) { _url = url; }

void DiscordSession::setBody(const std::string* body) { _body = body; }

void DiscordSession::setHeader(
    const std::vector<SleepyDiscord::HeaderPair>& header) {}

void DiscordSession::setMultipart(
    const std::initializer_list<SleepyDiscord::Part>& parts) {}

SleepyDiscord::Response DiscordSession::Post() {
  return request(SleepyDiscord::Post);
}

SleepyDiscord::Response DiscordSession::Patch() {
  return {SleepyDiscord::BAD_REQUEST, "", {}};
}

SleepyDiscord::Response DiscordSession::Delete() {
  return {SleepyDiscord::BAD_REQUEST, "", {}};
}

SleepyDiscord::Response DiscordSession::Get() {
  return request(SleepyDiscord::Get);
}

SleepyDiscord::Response DiscordSession::Put() {
  return {SleepyDiscord::BAD_REQUEST, "", {}};
}

size_t curl_response_callback(void* contents, size_t size, size_t nmemb,
                              std::string* s) {
  size_t newLength = size * nmemb;
  s->append((char*)contents, newLength);
  return newLength;
}

size_t curl_header_callback(
    char* buffer, size_t size, size_t nitems,
    std::map<std::string, std::string>* headers_response) {
  char buf_copy[nitems];
  strncpy(buf_copy, buffer, nitems);
  size_t key_size = 0;
  for (; key_size < nitems; ++key_size) {
    if (buf_copy[key_size] == ':') {
      buf_copy[key_size] = '\0';
      break;
    }
  }
  if (key_size == nitems) {
    return nitems * size;
  }
  buf_copy[nitems - 2] = '\0';  // Remove trailing new line
  std::string key(buf_copy);
  std::string value(buf_copy + key_size + 2);
  headers_response->insert(std::pair<std::string, std::string>(key, value));

  return nitems * size;
}

SleepyDiscord::Response DiscordSession::request(
    const SleepyDiscord::RequestMethod method) const {
  SleepyDiscord::Response response;

  CURL* curl = curl_easy_init();
  if (curl) {
    std::string s_response;
    std::map<std::string, std::string> headers_response;
    long response_code;
    curl_easy_setopt(curl, CURLOPT_URL,
                     _url.c_str());               // getting URL from char *url
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);  // useful for debugging
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,
                     0L);  // skipping cert. verification, if needed
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST,
                     0L);  // skipping hostname verification, if needed
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_response_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s_response);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers_response);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    if (method == SleepyDiscord::Post && _body) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, _body->c_str());
    }

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      response.text = std::move(s_response);
      response.header = std::move(headers_response);
      response.statusCode = response_code;
    } else {
      printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      response.statusCode = SleepyDiscord::BAD_REQUEST;
    }

    curl_easy_cleanup(curl);
  } else {
    printf("Couldn't initialize curl\n");
  }

  return response;
}
