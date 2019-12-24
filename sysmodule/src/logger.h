#pragma once

#define LOG_PATH "/config/nxcord"
#define LOG_NAME "nxcord-sys"

namespace Logger {
void write(const char* format, ...);
}
