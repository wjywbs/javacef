// Author Jingyi Wei. All rights reserved.

#ifndef CHROMIUM_SETTINGS_H
#define CHROMIUM_SETTINGS_H

#include <string>
#include <map>

struct ChromiumSettings {
  bool allow_right_button;

  std::map<std::string, std::string> cookies;
};

#endif //CHROMIUM_SETTINGS_H
