#include "nxcord_client.h"

NXCordClient::NXCordClient(const std::string& token) : DiscordClient(token) {}

void NXCordClient::onReady(SleepyDiscord::Ready readyData) {}

void NXCordClient::onMessage(SleepyDiscord::Message message) {}
