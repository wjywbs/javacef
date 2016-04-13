// Copyright (c) 2012 javacef Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "targetver.h"
#include "org_embedded_browser_Chromium.h"
#include "org_embedded_browser_DownloadWindow.h"
#include "jni_tools.h"

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <direct.h>
#include <sstream>
#include <string>
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_runnable.h"
#include "cefclient/cefclient.h"
#include "cefclient/client_handler.h"
#include "cefclient/client_switches.h"
#include "cefclient/cookie_handler.h"

char szWorkingDir[MAX_PATH];  // The current working directory
bool message_loop = false;
HWND mainBrowserHandle = NULL;

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1init
  (JNIEnv *env, jobject jobj, jlong hwnd, jstring url, jobject chromiumset)
{
  CefMainArgs main_args(GetModuleHandle(NULL));
  CefRefPtr<ClientApp> app(new ClientApp);

  // Retrieve the current working directory.
  if (_getcwd(szWorkingDir, MAX_PATH) == NULL)
    szWorkingDir[0] = 0;

  // Parse command line arguments. The passed in values are ignored on Windows.
  AppInitCommandLine(0, NULL);

  CefSettings settings;

  // Populate the settings based on command line arguments.
  AppGetSettings(settings);

  settings.multi_threaded_message_loop = message_loop;
  settings.log_severity = LOGSEVERITY_DISABLE;
  settings.no_sandbox = true;

  CefString path = CefString(szWorkingDir);

#if defined(WIN32)
#ifndef _WIN64
  CefString(&settings.browser_subprocess_path) = path.ToString() + "\\..\\cef_runtime\\win32\\cefclient.exe";
#else
  CefString(&settings.browser_subprocess_path) = path.ToString() + "\\..\\cef_runtime\\win64\\cefclient.exe";
#endif
#endif

  // Initialize CEF.
  CefInitialize(main_args, settings, app.get(), NULL);

  HWND hMain;
  hMain = (HWND)((void*)hwnd);

  const char* chr = env->GetStringUTFChars(url, 0);
  CefString wc = chr;

  CefRefPtr<ClientHandler> gh = InitBrowser(hMain, wc);
  gh->id = 1;

  env->ReleaseStringUTFChars(url, chr);

  // NOTE: This function (browser_init) should only be called ONCE in one process,
  // which means to call CefInitialize in the first broswer creation, and call 
  // CefShutdown in the exit of the process. Repeated calls to this function will 
  // NOT work, because the web page will not render. Most probably a bug in cef.
  set_jvm(env, jobj);
  send_handler(env, jobj, (jlong)(void*)gh);

  // Have to be here and use own jnienv to avoid errors.
  get_browser_settings(env, chromiumset, gh->csettings);

  if (!message_loop) {
    // Run the CEF message loop. This function will block until the application
    // recieves a WM_QUIT message.
    CefRunMessageLoop();
    cleanup_jvm(env);
    CefShutdown();
  }
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1new
  (JNIEnv *env, jobject jobj, jlong hwnd, jint id, jstring url, jobject chromiumset)
{
  const char* chr = env->GetStringUTFChars(url, 0);
  CefString wc = chr;
  CefRefPtr<ClientHandler> gh = NewBrowser((HWND)((void*)hwnd), wc);
  gh->id = id;
  env->ReleaseStringUTFChars(url, chr);
  send_handler(env, jobj, (jlong)(void*)gh);
  get_browser_settings(env, chromiumset, gh->csettings);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1close
  (JNIEnv *env, jobject jobj, jlong gh)
{
  CefRefPtr<ClientHandler> g_handler_local = (ClientHandler*)gh;
  /*CefRefPtr<ClientHandler> g_handler_local;
  _asm {
    push eax
    mov eax, gh
    mov g_handler_local, eax
    pop eax
  }*/

  if (g_handler_local.get() && g_handler_local->GetBrowser()) {
    g_handler_local->GetBrowser()->GetHost()->CloseBrowser(false);
  }
  //_asm mov g_handler_local, 0;
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1shutdown
  (JNIEnv *env, jobject jobj, jlong gh)
{
  CefRefPtr<ClientHandler> g_handler_local = (ClientHandler*)gh;

  if (g_handler_local.get() && g_handler_local->GetBrowser() &&
      g_handler_local->GetBrowser()->GetHost()->GetWindowHandle()) {
    g_handler_local->GetBrowser()->GetHost()->ParentWindowWillClose();
    g_handler_local->GetBrowser()->GetHost()->CloseBrowser(false);
    mainBrowserHandle = g_handler_local->GetBrowser()->GetHost()->GetWindowHandle();
  }
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1clean_1cookies
  (JNIEnv *env, jobject jobj)
{
  CefRefPtr<CefCookieManager> cookieManager = CefCookieManager::GetGlobalManager();
  CefRefPtr<CefCookieVisitor> cookieHandler = new CookieHandler(true);
  cookieManager->VisitAllCookies(cookieHandler);
  // Doesn't work:
  //CefString empty("");
  //cookieManager->DeleteCookies(empty, empty);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1setUrl
  (JNIEnv *env, jobject jobj, jlong gh, jstring url)
{
  CefRefPtr<ClientHandler> g_handler_local = (ClientHandler*)gh;
  const char* chr = env->GetStringUTFChars(url, 0);
  CefString urlString = chr;

  if (g_handler_local.get()) {
    CefRefPtr<CefBrowser> browser = g_handler_local->GetBrowser();
    if (browser.get() && browser->GetMainFrame())
      browser->GetMainFrame()->LoadURL(urlString);
  }
  env->ReleaseStringUTFChars(url, chr);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1resized
  (JNIEnv *env, jobject jobj, jlong gh, jlong hwnd)
{
  CefRefPtr<ClientHandler> g_handler_local = (ClientHandler*)gh;
  HWND hMain = (HWND)hwnd;

  if (g_handler_local.get() && g_handler_local->GetBrowser() &&
      g_handler_local->GetBrowser()->GetHost()->GetWindowHandle()) {
    // Resize the browser window and address bar to match the new frame window size
    RECT rect;
    GetClientRect(hMain, &rect);

    HDWP hdwp = BeginDeferWindowPos(1);
    hdwp = DeferWindowPos(hdwp, g_handler_local->GetBrowser()->GetHost()->GetWindowHandle(), NULL,
      rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
      SWP_NOZORDER);
    EndDeferWindowPos(hdwp);

    PostMessage(g_handler_local->GetBrowser()->GetHost()->GetWindowHandle(), WM_SETFOCUS, 0, NULL);
  }
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1back
  (JNIEnv *env, jobject jobj, jlong gh)
{
  CefRefPtr<ClientHandler> g_handler_local = (ClientHandler*)gh;
  if (g_handler_local.get()) {
    CefRefPtr<CefBrowser> browser = g_handler_local->GetBrowser();
    if (browser.get() && browser->CanGoBack()) {
      browser->GoBack();
    }
  }
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1forward
  (JNIEnv *env, jobject jobj, jlong gh)
{
  CefRefPtr<ClientHandler> g_handler_local = (ClientHandler*)gh;
  if (g_handler_local.get()) {
    CefRefPtr<CefBrowser> browser = g_handler_local->GetBrowser();
    if (browser.get() && browser->CanGoForward()) {
      browser->GoForward();
    }
  }
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1reload
  (JNIEnv *env, jobject jobj, jlong gh)
{
  CefRefPtr<ClientHandler> g_handler_local = (ClientHandler*)gh;
  if (g_handler_local.get()) {
    CefRefPtr<CefBrowser> browser = g_handler_local->GetBrowser();
    if (browser.get()) {
      browser->Reload();
    }
  }
}

JNIEXPORT jlong JNICALL Java_org_embedded_browser_DownloadWindow_getReceivedN
  (JNIEnv *env, jobject jobj, jint dhint)
{
  /*CefRefPtr<CefDownloadHandlerW> d_handler;
  _asm {
  push eax
  mov eax, dhint
  mov d_handler, eax
  pop eax
  }
  int64 rs = -10;
  if (d_handler.get())
    rs = d_handler->received_size;
  _asm {
    mov d_handler, 0
  }
  return rs;*/
  return 0;
}

// Global functions

std::string AppGetWorkingDirectory() {
  return szWorkingDir;
}

void AppQuitMessageLoop() {
  /*
    CefQuitMessageLoop();
  */
  if (mainBrowserHandle)
    PostMessage(mainBrowserHandle, WM_QUIT, 0, 0);
}
