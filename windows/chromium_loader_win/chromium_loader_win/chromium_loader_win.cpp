// Copyright (c) 2012 javacef Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"
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
#include "include/cef_nplugin.h"
#include "cefclient/cefclient.h"
#include "cefclient/client_handler.h"

char szWorkingdir[MAX_PATH];  // The current working directory

#define USE_MESSAGE_LOOP

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1init
  (JNIEnv *env, jobject jobj, jint hwnd, jstring url, jobject chromiumset)
{
  // Retrieve the current working directory.
  if (_getcwd(szWorkingdir, MAX_PATH) == NULL)
    szWorkingdir[0] = 0;

  // Parse command line arguments. The passed in values are ignored on Windows.
  AppInitCommandLine(0, NULL);

  CefSettings settings;
  CefRefPtr<CefApp> app;

  // Populate the settings based on command line arguments.
  AppGetSettings(settings, app);

#ifndef USE_MESSAGE_LOOP
  settings.multi_threaded_message_loop = true;
#endif

  // Initialize CEF.
  CefInitialize(settings, app);

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
  send_handler(env, jobj, (int)(void*)gh, true);

  // Have to be here and use own jnienv to avoid errors.
  get_browser_settings(env, chromiumset, gh->csettings);

  if (!settings.multi_threaded_message_loop) {
    // Run the CEF message loop. This function will block until the application
    // recieves a WM_QUIT message.
    CefRunMessageLoop();
  }

#ifdef USE_MESSAGE_LOOP
  CefShutdown();
#endif
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1new
  (JNIEnv *env, jobject jobj, jint hwnd, jint id, jstring url, jobject chromiumset)
{
  const char* chr = env->GetStringUTFChars(url, 0);
  CefString wc = chr;
  CefRefPtr<ClientHandler> gh = NewBrowser((HWND)((void*)hwnd), wc);
  gh->id = id;
  env->ReleaseStringUTFChars(url, chr);
  send_handler(env, jobj, (int)(void*)gh);
  get_browser_settings(env, chromiumset, gh->csettings);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1close
  (JNIEnv *env, jobject jobj, jint gh)
{
  CloseBrowser(gh);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1shutdown
  (JNIEnv *env, jobject jobj, jint gh)
{
  ShutdownBrowser(gh);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1clean_1cookies
  (JNIEnv *env, jobject jobj)
{
  CleanCookies();
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1setUrl
  (JNIEnv *env, jobject jobj, jint gh, jstring url)
{
  const char* chr = env->GetStringUTFChars(url, 0);
  CefString wc = chr;
  if (chr)
    setUrl(gh, wc);
  env->ReleaseStringUTFChars(url, chr);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1resized
  (JNIEnv *env, jobject jobj, jint gh, jint hwnd)
{
  resized(gh, (HWND)hwnd);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1back
  (JNIEnv *env, jobject jobj, jint gh)
{
  back(gh);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1forward
  (JNIEnv *env, jobject jobj, jint gh)
{
  forward(gh);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1reload
  (JNIEnv *env, jobject jobj, jint gh)
{
  reload(gh);
}

JNIEXPORT jlong JNICALL Java_org_embedded_browser_DownloadWindow_getReceivedN
  (JNIEnv *env, jobject jobj, jint dhint)
{
  CefRefPtr<CefDownloadHandlerW> d_handler;
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
  return rs;
}