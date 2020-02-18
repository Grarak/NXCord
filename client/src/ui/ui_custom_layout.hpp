#pragma once

#include <common/nxcord_com_interface.hpp>
#include <pu/Plutonium>
#include <utility>

#include "ui_result.hpp"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

class UICustomLayout : public pu::ui::Layout,
                       public UIResultContainer,
                       public std::enable_shared_from_this<UICustomLayout> {
 public:
  using Interface = std::shared_ptr<NXCordComInterface>;

 protected:
  Interface _interface;

  inline void onShowLayout(const std::shared_ptr<UICustomLayout> &layout) {
    if (_listener) {
      _listener->onShowLayout(shared_from_this(), layout);
    }
  }

 public:
  explicit UICustomLayout(Interface interface)
      : _interface(std::move(interface)) {}

  PU_SMART_CTOR(UICustomLayout)
};
