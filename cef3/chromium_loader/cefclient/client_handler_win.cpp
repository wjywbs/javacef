// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/client_handler.h"

#include <string>
#include <windows.h>
#include <shlobj.h> 

#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "cefclient/resource.h"
#include "chromium_loader/jni_tools.h"

void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    const CefString& url) {
  REQUIRE_UI_THREAD();
/*
  if (m_BrowserId == browser->GetIdentifier() && frame->IsMain())   {
    // Set the edit window text
    SetWindowText(m_EditHwnd, std::wstring(url).c_str());
  }
*/
}

void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  REQUIRE_UI_THREAD();

  // Set the frame window title bar
  CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
  if (m_BrowserId == browser->GetIdentifier())   {
    // Send title to java side.
    set_title(std::string(title).c_str(), browser->GetHost()->GetClient()->id);
  } else
    SetWindowText(GetAncestor(hwnd, GA_ROOT), std::wstring(title).c_str());
}

void ClientHandler::SendNotification(NotificationType type) {
  UINT id;
  switch (type) {
  case NOTIFY_CONSOLE_MESSAGE:
    id = ID_WARN_CONSOLEMESSAGE;
    break;
  case NOTIFY_DOWNLOAD_COMPLETE:
    id = ID_WARN_DOWNLOADCOMPLETE;
    break;
  case NOTIFY_DOWNLOAD_ERROR:
    id = ID_WARN_DOWNLOADERROR;
    break;
  default:
    return;
  }
  PostMessage(m_MainHwnd, WM_COMMAND, id, 0);
}

void ClientHandler::SetLoading(bool isLoading) {
  send_load(id, isLoading);
}

void ClientHandler::SetNavState(bool canGoBack, bool canGoForward) {
  // ignored
}

std::string ClientHandler::GetDownloadPath(const std::string& file_name) {
  TCHAR szFolderPath[MAX_PATH];
  std::string path;

  // Save the file in the user's "My Documents" folder.
  if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
                                NULL, 0, szFolderPath))) {
    path = CefString(szFolderPath);
    path += "\\" + file_name;
  }

  return path;
}
