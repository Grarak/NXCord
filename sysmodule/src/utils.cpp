#include "utils.h"

namespace Utils {
const time_t current_time_millis() {
  auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now());
  return ms.time_since_epoch().count();
}
}  // namespace Utils
