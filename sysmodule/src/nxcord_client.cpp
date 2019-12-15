#include "nxcord_client.h"

NXCordClient::NXCordClient(const std::string& token) : DiscordClient(token) {}

void NXCordClient::onReady(std::string* jsonMessage) {
}
