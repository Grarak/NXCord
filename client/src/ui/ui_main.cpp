#include "ui_main.hpp"

#include <common/logger.hpp>
#include <utility>

#include "ui_channels_layout.hpp"
#include "ui_connecting_layout.hpp"
#include "ui_logged_in.hpp"
#include "ui_login_2fa_layout.hpp"
#include "ui_login_layout.hpp"
#include "ui_result.hpp"
#include "ui_servers_layout.hpp"
#include "ui_settings_layout.hpp"

ResultListener::ResultListener(UIMain &uimain) : _ui_main(uimain) {}

void ResultListener::onResultLogin() { _ui_main.showLogin(); }

void ResultListener::onResultLoggedIn() { _ui_main.showLoggedIn(); }

void ResultListener::onResult2fa() { _ui_main.showLogin2fa(); }

void ResultListener::onResultConnecting() { _ui_main.showConnecting(); }

void ResultListener::onResultConnected() {
  _ui_main.showServers();
  _ui_main.CreateShowDialog(
      "Info",
      "A : Confirm\nB : Back\n- : Disconnect from current voice "
      "channel\n+ : Exit\nX: Disconnect\nY: Settings",
      {"Ok"}, false);
}

void ResultListener::onResultServerClick(
    const IPCStruct::DiscordServer &server) {
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
void ResultListener::onShowLayout(
    const std::shared_ptr<UICustomLayout> &current,
    const std::shared_ptr<UICustomLayout> &ui) {
  _ui_main.showLayout(current, ui);
}

UIMain::UIMain(pu::ui::render::Renderer::Ref renderer,
               UICustomLayout::Interface interface)
    : Application(std::move(renderer)),
      _interface(std::move(interface)),
      _listener(UIResultListener::New<ResultListener>(*this)) {}

void UIMain::OnLoad() {
  showLogin();

  AddThread([this]() {
    if (Utils::check_interval(_connection_looked_up, 1000) &&
        (_current_state == UIState::Connected ||
         _current_state == UIState::ShowChannels) &&
        _interface->isConnecting()) {
      showConnecting();
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
        case UIState::Connecting:
          _interface->stopConnection();
          showLoggedIn();
          break;
        case UIState::Connected:
          break;
        case UIState::ShowChannels:
        case UIState::Settings:
          showServers();
          break;
        case UIState::CustomLayout:
          _current_state = _previous_state;
          _current_layout = _previous_layout;
          LoadLayout(_current_layout);
          break;
      }
    } else if (Down & KEY_MINUS) {
      if (_interface->isConnectedVoiceChannel()) {
        if (CreateShowDialog("Confirm", "Disconnect from voice channel?",
                             {"Yes", "No"}, false) == 0) {
          _interface->disconnectVoiceChannel();
        }
      }
    } else if (Down & KEY_X) {
      switch (_current_state) {
        case UIState::Logged_in:
          _interface->logout();
          showLogin();
          break;
        case UIState::Connected:
        case UIState::ShowChannels:
          if (CreateShowDialog("Confirm", "Do you want to disconnect?",
                               {"Yes", "No"}, false) == 0) {
            _interface->stopConnection();
            showLoggedIn();
          }
          break;
        default:
          break;
      }
    } else if (Down & KEY_Y && (_current_state == UIState::Connected ||
                                _current_state == UIState::ShowChannels)) {
      showSettings();
    } else if (Down & KEY_PLUS) {
      Close();
    }
  });
}

void UIMain::showLogin() { loadCustomLayout<UILoginLayout>(UIState::Login); }

void UIMain::showLogin2fa() {
  loadCustomLayout<UILogin2faLayout>(UIState::Login_2fa);
}

void UIMain::showLoggedIn() {
  loadCustomLayout<UILoggedIn>(UIState::Logged_in);
}

void UIMain::showConnecting() {
  loadCustomLayout<UIConnectingLayout>(UIState::Connecting);
}

void UIMain::showServers() {
  loadCustomLayout<UIServersLayout>(UIState::Connected);
}

void UIMain::showChannels(const IPCStruct::DiscordServer &server) {
  loadCustomLayout<UIChannelsLayout>(UIState::ShowChannels, server);
}

void UIMain::showSettings() {
  loadCustomLayout<UISettingsLayout>(UIState ::Settings);
}

void UIMain::showLayout(const UICustomLayout::Ref &current,
                        const UICustomLayout::Ref &layout) {
  _previous_state = _current_state;
  _current_state = UIState::CustomLayout;
  _previous_layout = current;
  layout->setResultListener(_listener);
  LoadLayout(layout);
}

template <class Layout, class... Args>
void UIMain::loadCustomLayout(UIState state, Args &&... args) {
  static_assert(std::is_base_of<UICustomLayout, Layout>::value,
                "Layout must be child of UICustomLayout");

  Logger::write("UI: Setting state %lu\n", static_cast<size_t>(state));
  _current_state = state;
  _current_layout = Layout::New(_interface, std::forward<Args>(args)...);
  _current_layout->setResultListener(_listener);
  LoadLayout(_current_layout);
}
