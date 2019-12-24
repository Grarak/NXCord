#include <mbedtls/error.h>
#include <sys/socket.h>

#include "logger.h"
#include "mbedtls_wrapper.h"

std::string get_mbedtls_error(const char* name, int err) {
  char buf[128];
  mbedtls_strerror(err, buf, sizeof(buf));
  return buf;
}

MBedTLSWrapper::MBedTLSWrapper(const std::string& hostname) {
  mbedtls_entropy_init(&_entropy);
  mbedtls_ctr_drbg_init(&_ctr_drbg);
  mbedtls_x509_crt_init(&_cacert);
  mbedtls_net_init(&_net);
  mbedtls_ssl_init(&_ssl);
  mbedtls_ssl_config_init(&_ssl_conf);

  int ret;
  if ((ret = mbedtls_ctr_drbg_seed(&_ctr_drbg, mbedtls_entropy_func, &_entropy,
                                   nullptr, 0)) != 0) {
    _error = get_mbedtls_error("mbedtls_ctr_drbg_seed", ret);
    return;
  }

  if ((ret = mbedtls_ssl_config_defaults(&_ssl_conf, MBEDTLS_SSL_IS_CLIENT,
                                         MBEDTLS_SSL_TRANSPORT_STREAM,
                                         MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
    _error = get_mbedtls_error("mbedtls_ssl_config_defaults", ret);
    return;
  }

  mbedtls_ssl_conf_ca_chain(&_ssl_conf, &_cacert, nullptr);
  mbedtls_ssl_conf_rng(&_ssl_conf, mbedtls_ctr_drbg_random, &_ctr_drbg);
  mbedtls_ssl_conf_authmode(&_ssl_conf, MBEDTLS_SSL_VERIFY_OPTIONAL);

  if ((ret = mbedtls_ssl_setup(&_ssl, &_ssl_conf)) != 0) {
    _error = get_mbedtls_error("mbedtls_ssl_setup", ret);
    return;
  }

  mbedtls_ssl_set_hostname(&_ssl, hostname.c_str());

  _net.fd = -1;
}

MBedTLSWrapper::~MBedTLSWrapper() {
  if (_net.fd > 0) {
    Logger::write("Closing connection to %d\n", _net.fd);
    shutdown(_net.fd, SHUT_WR);
    close(_net.fd);
  }

  mbedtls_entropy_free(&_entropy);
  mbedtls_ctr_drbg_free(&_ctr_drbg);
  mbedtls_x509_crt_free(&_cacert);
  mbedtls_ssl_free(&_ssl);
  mbedtls_ssl_config_free(&_ssl_conf);
}

bool MBedTLSWrapper::usable() const { return _error.empty(); }

std::string MBedTLSWrapper::get_error() const { return _error; }

void MBedTLSWrapper::set_fd(int fd) {
  _net.fd = fd;
  mbedtls_ssl_set_bio(&_ssl, &_net, mbedtls_net_send, mbedtls_net_recv, NULL);
}

int MBedTLSWrapper::get_fd() const { return _net.fd; }

bool MBedTLSWrapper::start_ssl() {
  int ret = mbedtls_ssl_handshake(&_ssl);
  if (ret < 0) {
    _error = get_mbedtls_error("mbedtls_ssl_handshake", ret);
    return false;
  }
  return true;
}

int MBedTLSWrapper::write(const unsigned char* data, size_t data_size) {
  return mbedtls_ssl_write(&_ssl, data, data_size);
}

int MBedTLSWrapper::read(unsigned char* buf, size_t buf_size) {
  return mbedtls_ssl_read(&_ssl, buf, buf_size);
}
