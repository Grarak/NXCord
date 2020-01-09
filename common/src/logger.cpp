#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#include <cerrno>
#include <common/logger.hpp>
#include <common/utils.hpp>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>

namespace Logger {

extern std::string_view log_name;

std::mutex logger_mutex;

void write(const char* fmt, ...) {
  std::scoped_lock lock(logger_mutex);

  bool dir_exists = Utils::create_directories(LOG_PATH "/");
  FILE* fp = nullptr;
  int log_size;
  struct stat st;
  std::string full_path = LOG_PATH;
  full_path.append("/").append(log_name).append(".txt");

  if (dir_exists) {
    time_t unixTime = time(NULL);
    struct tm tStruct;
    localtime_r(&unixTime, &tStruct);

    fp = fopen(full_path.c_str(), "a");

    fprintf(fp, "%04i-%02i-%02i %02i:%02i:%02i: ", (tStruct.tm_year + 1900),
            tStruct.tm_mon, tStruct.tm_mday, tStruct.tm_hour, tStruct.tm_min,
            tStruct.tm_sec);
  }

  std::va_list va;
  va_start(va, fmt);
  if (fp) {
    std::vfprintf(fp, fmt, va);
  }
  std::vprintf(fmt, va);
  va_end(va);

  if (fp) {
    fstat(fileno(fp), &st);
    log_size = st.st_size;

    fclose(fp);

    if (log_size >= 1024 * 1024) {
      std::rename(
          full_path.c_str(),
          full_path.substr(0, full_path.size() - 4).append(".old.txt").c_str());
    }
  }
}
}  // namespace Logger
