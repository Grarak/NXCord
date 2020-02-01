#include <common/logger.hpp>

#include "ui_channels_layout.hpp"
#include "ui_connecting_layout.hpp"
#include "ui_login_2fa_layout.hpp"
#include "ui_login_layout.hpp"
#include "ui_main.hpp"
#include "ui_result.hpp"
#include "ui_servers_layout.hpp"

ResultListener::ResultListener(UIMain& uimain) : _ui_main(uimain) {}

void ResultListener::onResultLogin() { _ui_main.showLogin(); }

void ResultListener::onResultLoggedIn() { _ui_main.showConnecting(); }

void ResultListener::onResult2fa() { _ui_main.showLogin2fa(); }

void ResultListener::onResultConnected() {
  _ui_main.showServers();
  _ui_main.CreateShowDialog(
      "Info",
      "A : Confirm\nB : Back\n- : Disconnect from current voice "
      "channel\n+ : Exit",
      {"Ok"}, false);
}

void ResultListener::onResultServerClick(
    const IPCStruct::DiscordServer& server) {
  _ui_main.showChannels(server);
}

void ResultListener::onResultNoChannels() {
  _ui_main.showServers();
  _ui_main.CreateShowDialog("Error", "No voice channels found.", {"Ok"}, false);
}

bool ResultListener::onDialogJoinVoice() {
  return _ui_main.CreateShowDialog("Confirm",
                                   "Do you want to join this voice channel?",
                                   {"Yes", "No"}, false) == 0;
}

UIMain::UIMain(pu::ui::render::Renderer::Ref renderer,
               const UICustomLayout::Interface& interface)
    : Application(renderer),
      _interface(interface),
      _listener(UIResultListener::New<ResultListener>(*this)) {}

void UIMain::OnLoad() {
  showLogin();

  AddThread([this]() {
    time_t current_time = Utils::current_time_millis();
    // Check for connection every second
    if (_connection_looked_up == 0 ||
        current_time - _connection_looked_up >= 1000) {
      if ((_current_state == UIState::Connected ||
           _current_state == UIState::ShowChannels) &&
          !_interface->isConnected()) {
        showConnecting();
      }
      _connection_looked_up = current_time;
    }
  });

  SetOnInput([this](u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
    if (Down & KEY_B) {
      switch (_current_state) {
        case UIState::Login:
          break;
        case UIState::Login_2fa:
          showLogin();
          break;
        case UIState::Logged_in:
          break;
        case UIState::Connected:
          break;
        case UIState::ShowChannels:
          showServers();
          break;
      }
    } else if (Down & KEY_MINUS) {
      if (_interface->isConnectedVoiceChannel()) {
        if (CreateShowDialog("Confirm", "Disconnect from voice channel?",
                             {"Yes", "No"}, false) == 0) {
          _interface->disconnectVoiceChannel();
        }
      }
    } else if (Down & KEY_PLUS) {
      Close();
    }
  });
}

void UIMain::showLogin() { loadCustomLayout<UILoginLayout>(UIState::Login); }

void UIMain::showLogin2fa() {
  loadCustomLayout<UILogin2faLayout>(UIState::Login_2fa);
}

void UIMain::showConnecting() {
  loadCustomLayout<UIConnectingLayout>(UIState::Logged_in);
}

void UIMain::showServers() {
  loadCustomLayout<UIServersLayout>(UIState::Connected);
}

void UIMain::showChannels(const IPCStruct::DiscordServer& server) {
  loadCustomLayout<UIChannelsLayout>(UIState::ShowChannels, server);
}

template <class Layout, class... Args>
void UIMain::loadCustomLayout(UIState state, Args&&... args) {
  static_assert(std::is_base_of<UICustomLayout, Layout>::value,
                "Layout must be child of UICustomLayout");

  Logger::write("UI: Setting state %lu\n", static_cast<size_t>(state));
  _current_state = state;
  _current_layout = Layout::New(_interface, std::forward<Args>(args)...);
  _current_layout->setResultListener(_listener);
  LoadLayout(_current_layout);
}
