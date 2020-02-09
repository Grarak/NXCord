#pragma once

#include <common/nxcord_com_interface.hpp>
#include <pu/Plutonium>
#include <utility>

#include "ui_result.hpp"

class UICustomLayout : public pu::ui::Layout, public UIResultContainer {
 public:
  using Interface = std::shared_ptr<NXCordComInterface>;

 protected:
  Interface _interface;

 public:
  explicit UICustomLayout(Interface interface)
      : _interface(std::move(interface)) {}

  PU_SMART_CTOR(UICustomLayout)
};
