#pragma once

#include <memory>

class UIResultListener {
 protected:
  UIResultListener() {}

 public:
  template <class Listener, class... Args>
  static std::shared_ptr<Listener> New(Args&&... args) {
    static_assert(std::is_base_of<UIResultListener, Listener>::value,
                  "Listener must be child of UIResultListener");
    return std::shared_ptr<Listener>(new Listener(std::forward<Args>(args)...));
  }

  virtual void onResultLogin() = 0;
  virtual void onResultLoggedIn() = 0;
  virtual void onResult2fa() = 0;
  virtual void onResultConnected() = 0;
  virtual void onResultServerClick(const IPCStruct::DiscordServer& server) = 0;
  virtual void onResultNoChannels() = 0;
  virtual bool onDialogJoinVoice() = 0;
};

class UIResultContainer {
 private:
  std::shared_ptr<UIResultListener> _listener;

 public:
  inline void setResultListener(
      const std::shared_ptr<UIResultListener>& listener) {
    _listener = listener;
  }

#define DELEGATE_LISTENER(name) \
  inline void name() {          \
    if (_listener) {            \
      _listener->name();        \
    }                           \
  }

  DELEGATE_LISTENER(onResultLogin)
  DELEGATE_LISTENER(onResultLoggedIn)
  DELEGATE_LISTENER(onResult2fa)
  DELEGATE_LISTENER(onResultConnected)
  DELEGATE_LISTENER(onResultNoChannels)

  inline void onResultServerClick(const IPCStruct::DiscordServer& server) {
    if (_listener) {
      _listener->onResultServerClick(server);
    }
  }

  inline bool onDialogJoinVoice() {
    if (_listener) {
      return _listener->onDialogJoinVoice();
    }
    return false;
  }
};
