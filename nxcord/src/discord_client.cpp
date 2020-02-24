#include <switch.h>

#include <nxcord/base64.hpp>
#include <nxcord/discord_client.hpp>
#include <nxcord/discord_schedule_handler.hpp>
#include <nxcord/discord_session.hpp>
#include <nxcord/discord_udp_client.hpp>
#include <nxcord/discord_websocket.hpp>
#include <nxcord/mbedtls_wrapper.hpp>
#include <nxcord/opus_decoder.hpp>
#include <set>

#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

DiscordClient::DiscordClient() {
  setScheduleHandler<DiscordScheduleHandler>();
  // Create dummy variables here, so linker won't omit them
  DiscordUDPClient client;
  OpusDecoder opusDecoder(true);
  quit(false, true);
}

DiscordClient::~DiscordClient() { quit(); }

std::string create_acceptkey(const std::string &clientkey) {
  std::string s = clientkey + WS_GUID;
  unsigned char sha1[SHA1_HASH_SIZE];
  sha1CalculateHash(sha1, s.c_str(), s.size());
  return base64_encode(sha1, SHA1_HASH_SIZE);
}

bool DiscordClient::connect(const std::string &uri,
                            GenericMessageReceiver *message_processor,
                            SleepyDiscord::WebsocketConnection &connection) {
  unsigned char random_bytes[16];

  Logger::write("Generating random key\n");
  if (R_FAILED(csrngGetRandomBytes(random_bytes, 16))) {
    Logger::write("Failed to generate random bytes\n");
    message_processor->handleFailToConnect();
    return false;
  }

  std::string client_key = base64_encode(random_bytes, 16);

  bool zlib_compress = false;
  std::string real_uri = uri;
  std::string gateway = "wss://gateway.discord.gg";
  if (uri.substr(0, gateway.size()) == gateway) {
    // Enable compression for client websocket
    real_uri += "&encoding=json&compress=zlib-stream";
    zlib_compress = true;
  }

  DiscordSession session;
  session.setUrl(real_uri);

  std::vector<SleepyDiscord::HeaderPair> header = {
      {"Upgrade", "websocket"},
      {"Connection", "Upgrade"},
      {"Sec-WebSocket-Key", client_key},
      {"Sec-WebSocket-Version", "13"}};
  session.setHeader(header);

  Logger::write("Request connection to websocket\n");

  SleepyDiscord::Response response;
  std::unique_ptr<MBedTLSWrapper> mbedtls_wrapper =
      session.request(SleepyDiscord::Get, &response);

  if (response.statusCode !=
      SleepyDiscord::SWITCHING_PROTOCOLS) {  // error check
    Logger::write("Websocket connection Error: %s\n", response.text.c_str());
    message_processor->handleFailToConnect();
    return false;
  }

  auto it = response.header.find("sec-websocket-accept");
  if (it == response.header.end()) {
    Logger::write("Couldn't parse sec-websocket-accept\n");
    message_processor->handleFailToConnect();
    return false;
  }

  std::string accept_key = create_acceptkey(client_key);
  if (it->second != accept_key) {
    Logger::write("Accept key is invalid\n");
    message_processor->handleFailToConnect();
    return false;
  }

  Logger::write("Connection to websocket established\n");
  connection = std::make_shared<DiscordWebsocket>(
      message_processor, mbedtls_wrapper, zlib_compress);
  message_processor->initialize();
  return true;
}

void DiscordClient::send(std::string message,
                         SleepyDiscord::WebsocketConnection &connection) {
  if (connection) {
    auto discord_websocket =
        std::static_pointer_cast<DiscordWebsocket>(connection);
    std::scoped_lock lock(_websocket_send_mutex);
    int ret = discord_websocket->queue_message(message);
    if (ret != 0) {  // error
      Logger::write("Send error: ");
      switch (ret) {
        case WSLAY_ERR_NO_MORE_MSG:
          Logger::write("Could not queue given message\n");
          break;
        case WSLAY_ERR_INVALID_ARGUMENT:
          Logger::write("The given message is invalid\n");
          break;
        case WSLAY_ERR_NOMEM:
          Logger::write("Out of memory\n");
          break;
        default:
          Logger::write("unknown\n");
          break;
      }
    }
  }
}

void DiscordClient::disconnect(unsigned int code, const std::string reason,
                               SleepyDiscord::WebsocketConnection &connection) {
  Logger::write("Disconnecting client %s\n", reason.c_str());
  if (connection) {
    auto discord_websocket =
        std::static_pointer_cast<DiscordWebsocket>(connection);
    discord_websocket->disconnect(code, reason);
    connection.reset();
  }
}

void DiscordClient::onError(SleepyDiscord::ErrorCode errorCode,
                            const std::string error_message) {
  Logger::write("Error %i: %s\n", errorCode, error_message.c_str());
}

SleepyDiscord::Timer DiscordClient::schedule(SleepyDiscord::TimedTask code,
                                             const time_t milliseconds) {
  return static_cast<DiscordScheduleHandler &>(getScheduleHandler())
      .schedule(std::move(code), milliseconds);
}

void DiscordClient::stopClient() {
  static_cast<DiscordScheduleHandler &>(getScheduleHandler()).clear();
}

bool DiscordClient::setLoginCredentials(const std::string &email,
                                        const std::string &password,
                                        bool *has2fa, std::string *error) {
  quit();

  DiscordSession session;
  session.setUrl(std::string(loginURL));

  std::string request_body;
  request_body.reserve(28 + email.size() + password.size());
  request_body +=
      "{"
      "\"email\": \"";
  request_body += email;
  request_body +=
      "\","
      "\"password\":\"";
  request_body += password;
  request_body += "\"}";

  std::vector<SleepyDiscord::HeaderPair> header = {
      {"Content-Type", "application/json"}};
  session.setHeader(header);
  session.setBody(&request_body);

  SleepyDiscord::Response response = session.request(SleepyDiscord::Post);

  rapidjson::Document values;
  values.Parse(response.text.c_str(), response.text.length());

  if (!values.HasParseError()) {
    if (response.statusCode == SleepyDiscord::ErrorCode::OK) {
      bool sms = values.HasMember("sms") && values["sms"].GetBool();
      bool mfa = values.HasMember("mfa") && values["mfa"].GetBool();
      if (sms || mfa) {
        if (has2fa) {
          *has2fa = true;
        }
        _ticket = std::string(values["ticket"].GetString());
        return false;
      }
      _token = std::string(values["token"].GetString());
      return true;
    } else if (response.statusCode == SleepyDiscord::GENERAL_ERROR) {
      if (error) {
        *error = "Failed to establish connection";
        return false;
      }
    }

    if (error) {
      if (values.HasMember("email")) {
        *error = std::string(values["email"].GetArray()[0].GetString());
      } else if (values.HasMember("password")) {
        *error = std::string(values["password"].GetArray()[0].GetString());
      } else if (values.HasMember("captcha_key")) {
        *error = std::string(values["captcha_key"].GetArray()[0].GetString());
      }
    }
  } else if (error) {
    *error = "Failed to parse login response";
  }
  return false;
}

bool DiscordClient::submit2faTicket(const std::string &code) {
  if (_ticket.empty()) {
    return false;
  }

  DiscordSession session;
  session.setUrl(std::string(mfaURL));

  std::string request_body;
  request_body.reserve(23);
  request_body += R"({"code":")";
  request_body += code;
  request_body += R"(","ticket":")";
  request_body += _ticket;
  request_body += "\"}";
  request_body.reserve(23 + code.size() + _ticket.size());

  std::vector<SleepyDiscord::HeaderPair> header = {
      {"Content-Type", "application/json"}};
  session.setHeader(header);
  session.setBody(&request_body);

  SleepyDiscord::Response response = session.request(SleepyDiscord::Post);

  rapidjson::Document values;
  values.Parse(response.text.c_str(), response.text.length());

  if (!values.HasParseError() &&
      response.statusCode == SleepyDiscord::ErrorCode::OK) {
    _token = std::string(values["token"].GetString());
    return true;
  }
  return false;
}

void DiscordClient::startConnection() {
  if (!connection) {
    schedule([this]() { start(_token, 1); }, 0);
  }
}

void DiscordClient::tick() {
  if (connection) {
    auto discord_websocket =
        std::static_pointer_cast<DiscordWebsocket>(connection);
    discord_websocket->tick();
  }

  // Voice connections can remove themselves from the list
  // Consider this when looping through
  std::set<SleepyDiscord::VoiceContext *> already_called;
  size_t index = 0;
  while (index < voiceConnections.size()) {
    auto it = voiceConnections.begin();
    for (size_t i = 0; i < index; ++i) {
      ++it;
    }
    SleepyDiscord::VoiceConnection &voice_connection = *it;
    SleepyDiscord::VoiceContext &context = voice_connection.getContext();
    if (already_called.find(&context) != already_called.end()) {
      ++index;
    } else {
      if (voice_connection.connection) {
        auto voice_websocket = std::static_pointer_cast<DiscordWebsocket>(
            voice_connection.connection);
        voice_websocket->tick();
      }
      already_called.insert(&context);
      index = 0;
    }
  }

  static_cast<DiscordScheduleHandler &>(getScheduleHandler()).tick();
}
