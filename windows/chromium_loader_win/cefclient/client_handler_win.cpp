// Copyright (c) 2012 javacef Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"
#include "client_handler.h"
#include <string>
#include <map>
#include "../include/cef_browser.h"
#include "../include/cef_frame.h"
#include "client_popup_handler.h"
#include "resource.h"
#include "cookie_handler.h"

#include "jni_tools.h"

bool ClientHandler::OnBeforePopup(CefRefPtr<CefBrowser> parentBrowser,
                                  const CefPopupFeatures& popupFeatures,
                                  CefWindowInfo& windowInfo,
                                  const CefString& url,
                                  CefRefPtr<CefClient>& client,
                                  CefBrowserSettings& settings) {
  REQUIRE_UI_THREAD();
  if (!popupFeatures.dialog) {
    new_tab(client->id, std::string(url));
    windowInfo.m_dwStyle &= ~WS_VISIBLE;
    client = new ClientPopupHandler();
  }

  return false;
}

bool ClientHandler::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefRequest> request,
                                     CefString& redirectUrl,
                                     CefRefPtr<CefStreamReader>& resourceStream,
                                     CefRefPtr<CefResponse> response,
                                     int loadFlags) {
  REQUIRE_IO_THREAD();

  if (csettings.cookies.size() == 0)
    return false;

  CefRefPtr<CefCookieManager> ccm = CefCookieManager::GetGlobalManager();

  std::map<std::string, std::string>::iterator mit;
  for (mit = csettings.cookies.begin(); mit != csettings.cookies.end(); mit++)
  {
    CefCookie cookie;
    CefString(&cookie.name).FromString(mit->first);
    CefString(&cookie.value).FromString(mit->second);
    ccm->SetCookie(request->GetURL(), cookie);
  }

  return false;
}


void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    const CefString& url) {
 // REQUIRE_UI_THREAD();
}

void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  REQUIRE_UI_THREAD();

  // Set the frame window title bar
  CefWindowHandle hwnd = browser->GetWindowHandle();
  if (m_BrowserHwnd == hwnd) {
    // The frame window will be the parent of the browser window
    //hwnd = GetParent(hwnd);
    set_title(std::string(title).c_str(), browser->GetClient()->id);
  }
  else
    SetWindowText(hwnd, std::wstring(title).c_str());
}

void ClientHandler::SendNotification(NotificationType type) {

}

void ClientHandler::SetLoading(bool isLoading) {
  send_load(id, isLoading);
}

void ClientHandler::SetNavState(bool canGoBack, bool canGoForward) {

}

void ClientHandler::CloseMainWindow() {
  ::PostMessage(m_MainHwnd, WM_CLOSE, 0, 0);
}
