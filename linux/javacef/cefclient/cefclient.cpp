// Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefclient/cefclient.h"
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
#include "cefclient/client_switches.h"
#include "cefclient/string_util.h"
#include "cefclient/util.h"

CefRefPtr<ClientHandler> g_handler;
CefRefPtr<CefCommandLine> g_command_line;

CefRefPtr<ClientHandler> InitBrowser(CefWindowHandle handle, CefString url) {
  g_handler = new ClientHandler(); // set the first browser as the global main handler
  g_handler->SetMainHwnd(handle);
  g_handler->id = 1;

  CefWindowInfo info;
  CefBrowserSettings bsettings;

  // Initialize window info to the defaults for a child window
  info.SetAsChild(handle);

  // Create the new child browser window
  CefBrowserHost::CreateBrowserSync(info,
      static_cast<CefRefPtr<CefClient> >(g_handler),
	  url, bsettings);

  return g_handler;
}

CefRefPtr<ClientHandler> NewBrowser(CefWindowHandle handle, CefString url) {
  CefRefPtr<ClientHandler> g_handler_local = new ClientHandler();
  g_handler_local->SetMainHwnd(handle);

  CefWindowInfo info;
  CefBrowserSettings bsettings;

  // Initialize window info to the defaults for a child window
  info.SetAsChild(handle);

  // Create the new child browser window
  CefBrowserHost::CreateBrowserSync(info,
      static_cast<CefRefPtr<CefClient> >(g_handler_local),
	  url, bsettings);

  return g_handler_local;
}

void resized(jlong gh, CefWindowHandle handle) { // TODO: move to platform specific
  /*CefRefPtr<ClientHandler> g_handler_local;
  _asm {
    push eax
    mov eax, gh
    mov g_handler_local, eax
    pop eax
  }

  _asm mov g_handler_local, 0;*/
}

CefRefPtr<CefBrowser> AppGetBrowser() {
  if (!g_handler.get())
    return NULL;
  return g_handler->GetBrowser();
}

CefWindowHandle AppGetMainHwnd() {
  if (!g_handler.get())
    return NULL;
  return g_handler->GetMainHwnd();
}

void AppInitCommandLine(int argc, const char* const* argv) {
  g_command_line = CefCommandLine::CreateCommandLine();
#if defined(OS_WIN)
  g_command_line->InitFromString(::GetCommandLineW());
#else
  g_command_line->InitFromArgv(argc, argv);
#endif
}

// Returns the application command line object.
CefRefPtr<CefCommandLine> AppGetCommandLine() {
  return g_command_line;
}

// Returns the application settings based on command line arguments.
void AppGetSettings(CefSettings& settings) {
  const int MAX_PATH = 512;
  char workdir[MAX_PATH];
  if (!getcwd(workdir, sizeof(workdir)))
    return;

  ASSERT(g_command_line.get());
  if (!g_command_line.get())
    return;

#if defined(OS_WIN)
  settings.multi_threaded_message_loop =
      g_command_line->HasSwitch(cefclient::kMultiThreadedMessageLoop);
#endif

  CefString(&settings.cache_path) =
      g_command_line->GetSwitchValue(cefclient::kCachePath);

  CefString(&settings.resources_dir_path) = workdir;

  char localdir[MAX_PATH];
  strcpy(localdir, workdir);
  strcat(localdir, "/locales");// printf("%s\n", localdir);
  CefString(&settings.locales_dir_path) = localdir;

  // settings.single_process = true; // [FATAL:cookie_monster.cc(1178)] Check failed: !initialized_.
}

bool AppIsOffScreenRenderingEnabled() {
  ASSERT(g_command_line.get());
  if (!g_command_line.get())
    return false;

  return g_command_line->HasSwitch(cefclient::kOffScreenRenderingEnabled);
}

void RunGetSourceTest(CefRefPtr<CefBrowser> browser) {
  class Visitor : public CefStringVisitor {
   public:
    explicit Visitor(CefRefPtr<CefBrowser> browser) : browser_(browser) {}
    virtual void Visit(const CefString& string) OVERRIDE {
      std::string source = StringReplace(string, "<", "&lt;");
      source = StringReplace(source, ">", "&gt;");
      std::stringstream ss;
      ss << "<html><body>Source:<pre>" << source << "</pre></body></html>";
      browser_->GetMainFrame()->LoadString(ss.str(), "http://tests/getsource");
    }
   private:
    CefRefPtr<CefBrowser> browser_;
    IMPLEMENT_REFCOUNTING(Visitor);
  };

  browser->GetMainFrame()->GetSource(new Visitor(browser));
}

void RunGetTextTest(CefRefPtr<CefBrowser> browser) {
  class Visitor : public CefStringVisitor {
   public:
    explicit Visitor(CefRefPtr<CefBrowser> browser) : browser_(browser) {}
    virtual void Visit(const CefString& string) OVERRIDE {
      std::string text = StringReplace(string, "<", "&lt;");
      text = StringReplace(text, ">", "&gt;");
      std::stringstream ss;
      ss << "<html><body>Text:<pre>" << text << "</pre></body></html>";
      browser_->GetMainFrame()->LoadString(ss.str(), "http://tests/gettext");
    }
   private:
    CefRefPtr<CefBrowser> browser_;
    IMPLEMENT_REFCOUNTING(Visitor);
  };

  browser->GetMainFrame()->GetText(new Visitor(browser));
}

void RunRequestTest(CefRefPtr<CefBrowser> browser) {
  // Create a new request
  CefRefPtr<CefRequest> request(CefRequest::Create());

  // Set the request URL
  request->SetURL("http://tests/request");

  // Add post data to the request.  The correct method and content-
  // type headers will be set by CEF.
  CefRefPtr<CefPostDataElement> postDataElement(CefPostDataElement::Create());
  std::string data = "arg1=val1&arg2=val2";
  postDataElement->SetToBytes(data.length(), data.c_str());
  CefRefPtr<CefPostData> postData(CefPostData::Create());
  postData->AddElement(postDataElement);
  request->SetPostData(postData);

  // Add a custom header
  CefRequest::HeaderMap headerMap;
  headerMap.insert(
      std::make_pair("X-My-Header", "My Header Value"));
  request->SetHeaderMap(headerMap);

  // Load the request
  browser->GetMainFrame()->LoadRequest(request);
}

void RunPopupTest(CefRefPtr<CefBrowser> browser) {
  browser->GetMainFrame()->ExecuteJavaScript(
      "window.open('http://www.google.com');", "about:blank", 0);
}

void RunPluginInfoTest(CefRefPtr<CefBrowser> browser) {
  class Visitor : public CefWebPluginInfoVisitor {
   public:
    explicit Visitor(CefRefPtr<CefBrowser> browser)
        : browser_(browser) {
      html_ = "<html><head><title>Plugin Info Test</title></head><body>"
              "\n<b>Installed plugins:</b>";
    }
    ~Visitor() {
      html_ += "\n</body></html>";

      // Load the html in the browser.
      browser_->GetMainFrame()->LoadString(html_, "http://tests/plugin_info");
    }

    virtual bool Visit(CefRefPtr<CefWebPluginInfo> info, int count, int total)
        OVERRIDE {
      html_ +=  "\n<br/><br/>Name: " + info->GetName().ToString() +
                "\n<br/>Description: " + info->GetDescription().ToString() +
                "\n<br/>Version: " + info->GetVersion().ToString() +
                "\n<br/>Path: " + info->GetPath().ToString();
      return true;
    }

   private:
    std::string html_;
    CefRefPtr<CefBrowser> browser_;
    IMPLEMENT_REFCOUNTING(Visitor);
  };

  CefVisitWebPluginInfo(new Visitor(browser));
}

void RunLocalStorageTest(CefRefPtr<CefBrowser> browser) {
  browser->GetMainFrame()->LoadURL("http://tests/localstorage");
}

void RunAccelerated2DCanvasTest(CefRefPtr<CefBrowser> browser) {
  browser->GetMainFrame()->LoadURL(
      "http://mudcu.be/labs/JS1k/BreathingGalaxies.html");
}

void RunAcceleratedLayersTest(CefRefPtr<CefBrowser> browser) {
  browser->GetMainFrame()->LoadURL(
      "http://webkit.org/blog-files/3d-transforms/poster-circle.html");
}

void RunWebGLTest(CefRefPtr<CefBrowser> browser) {
  browser->GetMainFrame()->LoadURL(
      "http://webglsamples.googlecode.com/hg/field/field.html");
}

void RunHTML5VideoTest(CefRefPtr<CefBrowser> browser) {
  browser->GetMainFrame()->LoadURL(
      "http://www.youtube.com/watch?v=siOHh0uzcuY&html5=True");
}

void RunXMLHTTPRequestTest(CefRefPtr<CefBrowser> browser) {
  browser->GetMainFrame()->LoadURL("http://tests/xmlhttprequest");
}

void RunDragDropTest(CefRefPtr<CefBrowser> browser) {
  browser->GetMainFrame()->LoadURL("http://html5demos.com/drag");
}

void RunGeolocationTest(CefRefPtr<CefBrowser> browser) {
  browser->GetMainFrame()->LoadURL("http://html5demos.com/geo");
}

void RunTransparencyTest(CefRefPtr<CefBrowser> browser) {
  browser->GetMainFrame()->LoadURL("http://tests/transparency");
}
