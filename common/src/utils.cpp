#include <dirent.h>

#include <common/utils.hpp>
#include <cstring>
#include <fstream>

namespace Utils {

const time_t current_time_millis() {
  auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now());
  return ms.time_since_epoch().count();
}

const bool create_directories(const char* file_path) {
  DIR* log_dir = opendir(file_path);
  if (log_dir) {
    closedir(log_dir);
    return true;
  }

  for (const char* p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
    std::string path(file_path, p - file_path);
    if (mkdir(path.c_str(), 0755) == -1) {
      if (errno != EEXIST) {
        return false;
      }
    }
  }
  mkdir(file_path, 0755);
  return true;
}

const void copy_file(const std::string& srcPath, const std::string& destPath) {
  std::ifstream src(srcPath, std::ios::binary);
  std::ofstream dest(destPath, std::ios::binary);

  dest << src.rdbuf();

  src.close();
  dest.flush();
  dest.close();
}

const bool file_exists(const std::string& path) {
  return access(path.c_str(), F_OK) != -1;
}

const bool check_interval(time_t& previous_time, time_t interval) {
  time_t current = current_time_millis();
  if (previous_time == 0 || current - previous_time >= interval) {
    previous_time = current;
    return true;
  }
  return false;
}

}  // namespace Utils
