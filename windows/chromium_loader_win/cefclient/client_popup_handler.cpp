// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"
#include "client_popup_handler.h"
#include "../include/cef_frame.h"
#include "util.h"

ClientPopupHandler::ClientPopupHandler() {
}

ClientPopupHandler::~ClientPopupHandler() {
}

bool ClientPopupHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        NavType navType,
                                        bool isRedirect) {
  REQUIRE_UI_THREAD();

  browser->CloseBrowser();
  return true;
}
