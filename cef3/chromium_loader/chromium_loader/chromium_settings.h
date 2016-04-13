// Copyright (c) 2012 javacef Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CHROMIUM_SETTINGS_H
#define CHROMIUM_SETTINGS_H
#pragma once

#include <string>
#include <map>

struct ChromiumSettings {
  bool allow_right_button;

  std::map<std::string, std::string> cookies;
};

#endif // CHROMIUM_SETTINGS_H
