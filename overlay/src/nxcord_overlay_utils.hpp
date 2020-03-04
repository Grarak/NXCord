#pragma once

#define private public
#define protected public
#include <tesla.hpp>
#undef private
#undef protected

constexpr std::string_view NO_SYSMODULE = "Can't communicate with \nsysmodule!";
constexpr std::string_view NOT_CONNECTED = "Not connected to voice channel.";

inline tsl::elm::OverlayFrame *defaultOverlayFrame() {
  return new tsl::elm::OverlayFrame("NXCord", NXCORD_VERSION);
}
