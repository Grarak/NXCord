#define TESLA_INIT_IMPL
#define private public
#define protected public
#include <tesla.hpp>
#undef private
#undef protected

#include "nxcord_overlay.hpp"

namespace Logger {
std::string_view log_name = "nxcord-overlay";
}

int main(int argc, char *argv[]) {
  return tsl::loop<NXCordOverlay>(argc, argv);
}