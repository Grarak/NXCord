#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

#include "logger.h"

namespace Logger {

bool create_directories(const char* file_path) {
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

void write(const char* fmt, ...) {
  DIR* log_dir = opendir(LOG_PATH);
  FILE* fp = nullptr;
  int log_size;
  struct stat st;

  if (!log_dir && create_directories(LOG_PATH "/")) {
    log_dir = opendir(LOG_PATH);
  }

  if (log_dir) {
    time_t unixTime = time(NULL);
    struct tm tStruct;
    localtime_r(&unixTime, &tStruct);

    fp = fopen(LOG_PATH "/" LOG_NAME ".txt", "a");

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
    closedir(log_dir);

    if (log_size >= 1024 * 1024) {
      std::rename(LOG_PATH "/" LOG_NAME ".txt",
                  LOG_PATH "/" LOG_NAME ".old.txt");
    }
  }
}
}  // namespace Logger
