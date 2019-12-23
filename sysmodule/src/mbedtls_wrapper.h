#pragma once
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/x509_crt.h>

#include <string>

class MBedTLSWrapper {
 private:
  std::string _error;

  mbedtls_entropy_context _entropy;
  mbedtls_ctr_drbg_context _ctr_drbg;
  mbedtls_x509_crt _cacert;
  mbedtls_net_context _net;
  mbedtls_ssl_context _ssl;
  mbedtls_ssl_config _ssl_conf;

 public:
  MBedTLSWrapper(const std::string &hostname);
  ~MBedTLSWrapper();

  bool usable() const;
  std::string get_error() const;

  void set_fd(int fd);
  int get_fd() const;
  bool start_ssl();
  int write(const unsigned char *data, size_t data_size);
  int read(unsigned char *buf, size_t buf_size);
};
