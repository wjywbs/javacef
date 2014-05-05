// Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <gtk/gtk.h>
#include <string>
#include "cefclient/client_handler.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "cefclient/cefclient.h"
#include "chromium_loader/jni_tools.h"

void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    const CefString& url) {
  REQUIRE_UI_THREAD();

  if (m_BrowserId == browser->GetIdentifier() && frame->IsMain()) {
      // Set the edit window text
    /*std::string urlStr(url);
    gtk_entry_set_text(GTK_ENTRY(m_EditHwnd), urlStr.c_str());*/
  }
}

void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
                                  const CefString& title) {
  REQUIRE_UI_THREAD();

  std::string titleStr(title);
  if (m_BrowserId == browser->GetIdentifier()) {
    if (id != browser->GetHost()->GetClient()->id)
      fprintf(stderr, "ClientHandler::OnTitleChange id is not the same\n");

    // Send title to java side if the browser is not closed.
    if (id != -1)
      set_title(std::string(title).c_str(), id);
  } else {
    GtkWidget* window = gtk_widget_get_ancestor(
        GTK_WIDGET(browser->GetHost()->GetWindowHandle()),
        GTK_TYPE_WINDOW);
    gtk_window_set_title(GTK_WINDOW(window), titleStr.c_str());
  }
}

void ClientHandler::SendNotification(NotificationType type) {
  // TODO(port): Implement this method.
  printf("[javacef] Notification status: %d\n", type);
}

void ClientHandler::SetLoading(bool isLoading) {
  if (id != -1)
    send_load(id, isLoading);
}

void ClientHandler::SetNavState(bool canGoBack, bool canGoForward) {
  // Do nothing.
}

std::string ClientHandler::GetDownloadPath(const std::string& file_name) {
  return std::string();
}
