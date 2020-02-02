#pragma once

#include <common/nxcord_com_interface.hpp>
#include <common/utils.hpp>
#include <pu/Plutonium>

#include "ui_custom_layout.hpp"

class UIMain;

class ResultListener : public UIResultListener {
 private:
  UIMain& _ui_main;
  ResultListener(UIMain& uimain);

  template <class Listener, class... Args>
  friend std::shared_ptr<Listener> UIResultListener::New(Args&&... args);

 public:
  void onResultLogin() override;
  void onResultLoggedIn() override;
  void onResult2fa() override;
  void onResultConnecting() override;
  void onResultConnected() override;
  void onResultServerClick(const IPCStruct::DiscordServer& server) override;
  void onResultNoChannels();
  bool onDialogJoinVoice();
};

class UIMain : public pu::ui::Application {
 private:
  enum class UIState {
    Login,
    Login_2fa,
    Logged_in,
    Connecting,
    Connected,
    ShowChannels,
  };

  UIState _current_state = UIState::Login;

  UICustomLayout::Interface _interface;
  std::shared_ptr<ResultListener> _listener;

  UICustomLayout::Ref _current_layout;
  time_t _connection_looked_up = 0;

  void showLogin();
  void showLogin2fa();
  void showLoggedIn();
  void showConnecting();
  void showServers();
  void showChannels(const IPCStruct::DiscordServer& server);

  template <class Layout, class... Args>
  void loadCustomLayout(UIState state, Args&&... args);

  friend ResultListener;

 public:
  using Application::Application;

  UIMain(pu::ui::render::Renderer::Ref renderer,
         const UICustomLayout::Interface& interface);

  PU_SMART_CTOR(UIMain)

  void OnLoad() override;
};
