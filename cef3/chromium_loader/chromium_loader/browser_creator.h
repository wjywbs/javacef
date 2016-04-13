// Copyright (c) 2016 javacef Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CHROMIUM_LOADER_BROWSER_CREATOR_H
#define CHROMIUM_LOADER_BROWSER_CREATOR_H
#pragma once

#include "include/cef_base.h"
#include "cefclient/browser/client_handler.h"

CefRefPtr<client::ClientHandler> NewBrowser(CefWindowHandle handle,
                                            CefString url);
void GetBrowserWindowInfo(CefWindowInfo& info, CefWindowHandle handle);

#endif  // CHROMIUM_LOADER_BROWSER_CREATOR_H
