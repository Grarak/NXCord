#include "error_gui.hpp"

ErrorGui::ErrorGui(std::string error) : _error(std::move(error)) {}

tsl::elm::Element *ErrorGui::createUI() {
  auto rootFrame = defaultOverlayFrame();

  rootFrame->setContent(new tsl::elm::CustomDrawer(
      [this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
        renderer->drawString(_error.c_str(), false, x, y, 25,
                             tsl::gfx::Color(0xff, 0xff, 0xff, 0xff));
      }));

  return rootFrame;
}
