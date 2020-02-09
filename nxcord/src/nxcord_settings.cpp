#include <common/utils.hpp>
#include <nxcord/nxcord_settings.hpp>

NXCordSettings::NXCordSettings() {
  Utils::create_directories(CONFIG_PATH "/");
  _file_path = CONFIG_PATH "/" CONFIG_NAME;

  _ini_instance = std::unique_ptr<simpleIniParser::Ini>(
      simpleIniParser::Ini::parseFile(_file_path));
  if (!_ini_instance) {
    _ini_instance = std::make_unique<simpleIniParser::Ini>();
  }
}

std::unique_ptr<NXCordSettings> NXCordSettings::New() {
  return std::unique_ptr<NXCordSettings>(new NXCordSettings);
}
