#include <switch.h>

#include "base64.h"
#include "discord_client.h"
#include "discord_schedule_handler.h"
#include "discord_session.h"
#include "discord_websocket.h"
#include "logger.h"
#include "mbedtls_wrapper.h"

#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

DiscordClient::DiscordClient(const std::string &token) : _token(token) {
  setScheduleHandler<DiscordScheduleHandler>(this);
  start(_token, 1);
}

DiscordClient::~DiscordClient() { quit(false); }

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

  DiscordSession session;
  session.setUrl(uri);

  std::vector<SleepyDiscord::HeaderPair> header = {
      {"Upgrade", "websocket"},
      {"Connection", "Upgrade"},
      {"Sec-WebSocket-Key", client_key},
      {"Sec-WebSocket-Version", "13"}};
  session.setHeader(header);

  Logger::write("Request connection to websocket\n");

  SleepyDiscord::Response response;
  std::unique_ptr<MBedTLSWrapper> mbedtls_wrapper =
      std::move(session.request(SleepyDiscord::Get, &response));

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
  connection =
      std::make_shared<DiscordWebsocket>(message_processor, mbedtls_wrapper);
  return true;
}

void DiscordClient::send(std::string message,
                         SleepyDiscord::WebsocketConnection &connection) {
  if (connection) {
    auto discord_websocket =
        std::static_pointer_cast<DiscordWebsocket>(connection);
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

void DiscordClient::tick() {
  if (connection) {
    auto discord_websocket =
        std::static_pointer_cast<DiscordWebsocket>(connection);
    if (!discord_websocket->tick()) {
      static_cast<DiscordScheduleHandler &>(getScheduleHandler()).clear();
      restart();
      return;
    }
  }

  for (auto &voice_connection : voiceConnections) {
    if (voice_connection.connection) {
      auto voice_websocket = std::static_pointer_cast<DiscordWebsocket>(
          voice_connection.connection);
      voice_websocket->tick();
    }
  }

  static_cast<DiscordScheduleHandler &>(getScheduleHandler()).tick();
}
