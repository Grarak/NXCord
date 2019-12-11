#pragma once
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/ssl.h>
#include <mbedtls/x509_crt.h>

class MBedTLSWrapper {
 private:
  std::string _error;

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

  void register_send_receive(void *p_bio, mbedtls_ssl_send_t *f_send,
                             mbedtls_ssl_recv_t *f_recv);
  bool start_ssl();
  bool write(const char *data, size_t data_size);
  size_t read(char *buf, size_t buf_size);
};
