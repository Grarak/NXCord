#pragma once
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/ssl.h>
#include <mbedtls/x509_crt.h>

#include <string>

class MBedTLSWrapper {
 private:
  std::string _error;
  int _fd = -1;

  mbedtls_entropy_context _entropy;
  mbedtls_ctr_drbg_context _ctr_drbg;
  mbedtls_x509_crt _cacert;
  mbedtls_ssl_context _ssl;
  mbedtls_ssl_config _ssl_conf;

 public:
  MBedTLSWrapper(const std::string &hostname);
  ~MBedTLSWrapper();

  bool usable() const;
  std::string get_error() const;

  void set_fd(int fd);
  bool start_ssl();
  bool write(const char *data, size_t data_size);
  size_t read(char *buf, size_t buf_size);
};
