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

  mbedtls_entropy_context _entropy{};
  mbedtls_ctr_drbg_context _ctr_drbg{};
  mbedtls_x509_crt _cacert{};
  mbedtls_net_context _net{};
  mbedtls_ssl_context _ssl{};
  mbedtls_ssl_config _ssl_conf{};

 public:
  explicit MBedTLSWrapper(const std::string &hostname);

  ~MBedTLSWrapper();

  [[nodiscard]] inline bool usable() const { return _error.empty(); }

  [[nodiscard]] inline std::string getError() const { return _error; }

  void setFd(int fd);

  [[nodiscard]] inline int getFd() const { return _net.fd; }

  bool startSSL();

  int write(const unsigned char *data, size_t data_size);

  int read(unsigned char *buf, size_t buf_size);
};
