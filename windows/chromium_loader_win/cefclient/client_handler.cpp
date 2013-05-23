// Copyright (c) 2012 javacef Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"
#include "client_handler.h"
#include <stdio.h>
#include <sstream>
#include <string>
#include "../include/cef_browser.h"
#include "../include/cef_frame.h"
#include "../include/cef_cookie.h"
#include "cefclient.h"
#include "download_handler.h"

#include "jni_tools.h"

ClientHandler::ClientHandler()
  : m_MainHwnd(NULL),
    m_BrowserHwnd(NULL),
    m_bFormElementHasFocus(false) {
}

ClientHandler::~ClientHandler() {
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  REQUIRE_UI_THREAD();

  AutoLock lock_scope(this);
  if (!m_Browser.get()) {
    // We need to keep the main child window, but not popup windows
    m_Browser = browser;
    m_BrowserHwnd = browser->GetWindowHandle();
  }
}

bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser) {
  REQUIRE_UI_THREAD();

  if (m_BrowserHwnd == browser->GetWindowHandle()) {
    // Since the main window contains the browser window, we need to close
    // the parent window instead of the browser window.
    CloseMainWindow();

    // Return true here so that we can skip closing the browser window
    // in this pass. (It will be destroyed due to the call to close
    // the parent above.)
    return true;
  }

  // A popup browser window is not contained in another window, so we can let
  // these windows close by themselves.
  return false;
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  //REQUIRE_FILE_THREAD();//!!!need to fix//if (!g_handler.get()) sethandler(gh);

  if (m_BrowserHwnd == browser->GetWindowHandle()) {
    // Free the browser pointer so that the browser can be destroyed
    m_Browser = NULL;
  }
}

void ClientHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame) {
  REQUIRE_UI_THREAD();

  if (m_BrowserHwnd == browser->GetWindowHandle() && frame->IsMain()) {
    // We've just started loading a page
    SetLoading(true);
  }
}

void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              int httpStatusCode) {
  REQUIRE_UI_THREAD();

  if (m_BrowserHwnd == browser->GetWindowHandle() && frame->IsMain()) {
    // We've just finished loading a page
    SetLoading(false);

	if (!csettings.allow_right_button)
	  frame->ExecuteJavaScript("document.body.oncontextmenu=function(){return false;};", frame->GetURL(), 0);
  }
}

bool ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& failedUrl,
                                CefString& errorText) {
  REQUIRE_UI_THREAD();

  if (errorCode == ERR_CACHE_MISS) {
    // Usually caused by navigating to a page with POST data via back or
    // forward buttons.
    errorText = "<html><head><title>Expired Form Data</title></head>"
                "<body><h1>Expired Form Data</h1>"
                "<h2>Your form request has expired. "
                "Click reload to re-submit the form data.</h2></body>"
                "</html>";
  } else {
    // All other messages.
    std::stringstream ss;
    ss <<       "<html><head><title>Load Failed</title></head>"
                "<body><h1>Load Failed</h1>"
                "<h2>Load of URL " << std::string(failedUrl) <<
                " failed with error code " << static_cast<int>(errorCode) <<
                ".</h2></body>"
                "</html>";
    errorText = ss.str();
  }

  return false;
}

bool ClientHandler::GetDownloadHandler(CefRefPtr<CefBrowser> browser,
                                       const CefString& mimeType,
                                       const CefString& fileName,
                                       int64 contentLength,
                                       CefRefPtr<CefDownloadHandler>& handler) {
  REQUIRE_UI_THREAD();

  jobject jdw = get_download_window(std::string(fileName).c_str(), contentLength, std::string(mimeType).c_str());//wstring?
  std::string path = get_download_path_init(jdw);
  if (path.length() > 0) {
    // Create the handler for the file download.
    handler = CreateDownloadHandler(this, path, jdw);
    send_download_handler(jdw, (int)(void*)handler);
  }

  // Close the browser window if it is a popup with no other document contents.
  if (browser->IsPopup() && !browser->HasDocument())
    browser->CloseBrowser();
  if (!browser->HasDocument())
    close_tab(id);

  return true;
}

void ClientHandler::OnNavStateChange(CefRefPtr<CefBrowser> browser,
                                     bool canGoBack,
                                     bool canGoForward) {
  REQUIRE_UI_THREAD();

  //SetNavState(canGoBack, canGoForward);
  send_navstate(id, canGoBack, canGoForward);
}

bool ClientHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                     const CefString& message,
                                     const CefString& source,
                                     int line) {

  return false;
}

void ClientHandler::OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         CefRefPtr<CefDOMNode> node) {
  REQUIRE_UI_THREAD();

  // Set to true if a form element has focus.
  m_bFormElementHasFocus = (node.get() && node->IsFormControlElement());
}

bool ClientHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser,
                               KeyEventType type,
                               int code,
                               int modifiers,
                               bool isSystemKey,
                               bool isAfterJavaScript) {
  REQUIRE_UI_THREAD();

  return false;
}

bool ClientHandler::GetPrintHeaderFooter(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         const CefPrintInfo& printInfo,
                                         const CefString& url,
                                         const CefString& title,
                                         int currentPage,
                                         int maxPages,
                                         CefString& topLeft,
                                         CefString& topCenter,
                                         CefString& topRight,
                                         CefString& bottomLeft,
                                         CefString& bottomCenter,
                                         CefString& bottomRight) {
  REQUIRE_UI_THREAD();

  // Place the page title at top left
  topLeft = title;
  // Place the page URL at top right
  topRight = url;

  // Place "Page X of Y" at bottom center
  std::stringstream strstream;
  strstream << "Page " << currentPage << " of " << maxPages;
  bottomCenter = strstream.str();

  return false;
}

void ClientHandler::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefV8Context> context) {
  REQUIRE_UI_THREAD();
}

bool ClientHandler::OnDragStart(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefDragData> dragData,
                                DragOperationsMask mask) {
  //REQUIRE_UI_THREAD();

  return false;
}

bool ClientHandler::OnDragEnter(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefDragData> dragData,
                                DragOperationsMask mask) {
  //REQUIRE_UI_THREAD();

  return false;
}

bool ClientHandler::OnBeforeScriptExtensionLoad(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const CefString& extensionName) {
  return false;
}

void ClientHandler::NotifyDownloadComplete(const CefString& fileName) {

}

void ClientHandler::NotifyDownloadError(const CefString& fileName) {

}

void ClientHandler::SetMainHwnd(CefWindowHandle hwnd) {
  AutoLock lock_scope(this);
  m_MainHwnd = hwnd;
}

void ClientHandler::SetEditHwnd(CefWindowHandle hwnd) {

}

void ClientHandler::SetButtonHwnds(CefWindowHandle backHwnd,
                                   CefWindowHandle forwardHwnd,
                                   CefWindowHandle reloadHwnd,
                                   CefWindowHandle stopHwnd) {

}
