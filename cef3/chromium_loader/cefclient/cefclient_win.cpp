// Copyright (c) 2013 javacef Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/cefclient.h"
#include <windows.h>

#include <stdio.h>
#include <cstdlib>
#include <sstream>
#include <string>
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_frame.h"
#include "include/cef_runnable.h"
#include "include/cef_web_plugin.h"
#include "cefclient/client_handler.h"
#include "cefclient/client_switches.h"
#include "cefclient/string_util.h"
#include "cefclient/util.h"

// The global ClientHandler reference.
extern CefRefPtr<ClientHandler> g_handler;

CefRefPtr<ClientHandler> InitBrowser(CefWindowHandle handle, CefString url) {
  // Set the first browser as the global main handler
  g_handler = new ClientHandler();
  g_handler->id = 1;

  CreateBrowser(handle, url, g_handler);
  return g_handler;
}

CefRefPtr<ClientHandler> NewBrowser(CefWindowHandle handle, CefString url) {
  CefRefPtr<ClientHandler> g_handler_local = new ClientHandler();
  CreateBrowser(handle, url, g_handler_local);
  return g_handler_local;
}

void CreateBrowser(CefWindowHandle handle, CefString url, CefRefPtr<ClientHandler> g_handler_local) {
  g_handler_local->SetMainHwnd(handle);

  RECT rect;
  GetClientRect(handle, &rect);

  CefWindowInfo info;
  CefBrowserSettings bsettings;

  // Initialize window info to the defaults for a child window
  info.SetAsChild(handle, rect);

  // Create the new child browser window
  CefBrowserHost::CreateBrowser(info,
      static_cast<CefRefPtr<CefClient> >(g_handler_local),
	  url, bsettings, NULL);
}