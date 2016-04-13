// Copyright (c) 2016 javacef Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "chromium_loader/browser_creator.h"

#include "include/cef_base.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/cef_request_context.h"
#include "cefclient/browser/client_handler.h"
#include "cefclient/browser/client_handler_std.h"

CefRefPtr<client::ClientHandler> NewBrowser(CefWindowHandle handle,
                                            CefString url) {
  CefRefPtr<client::ClientHandler> g_handler_local =
      new client::ClientHandlerStd(NULL, url);

  CefWindowInfo info;
  CefBrowserSettings bsettings;
  CefRequestContextSettings rsettings;

  GetBrowserWindowInfo(info, handle);

  // Create the new child browser window
  CefBrowserHost::CreateBrowser(info, g_handler_local, url, bsettings, NULL);
  return g_handler_local;
}
