#define TESLA_INIT_IMPL
#include <tesla.hpp>

class NXCordGui : public tsl::Gui {
 public:
  tsl::elm::Element* createUI() override {
    auto rootFrame = new tsl::elm::OverlayFrame("NXCord", NXCORD_VERSION);
    return rootFrame;
  }

  void update() override {}

  bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput,
                   JoystickPosition leftJoyStick,
                   JoystickPosition rightJoyStick) override {
    return false;
  }
};

class NXCordOverlay : public tsl::Overlay<NXCordGui> {
 public:
  void initServices() override {}
  void exitServices() override {}

  void onShow() override {}
  void onHide() override {}
};

int main(int argc, char* argv[]) {
  return tsl::loop<NXCordOverlay>(argc, argv);
}