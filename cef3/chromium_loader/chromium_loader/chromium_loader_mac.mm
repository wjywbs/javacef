// Copyright (c) 2012 javacef Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <objc/runtime.h>
#include "org_embedded_browser_Chromium.h"
#include "org_embedded_browser_DownloadWindow.h"
#include "jni_tools.h"

#include <iostream>
#include <sstream>
#include <string>
#include "include/cef_app.h"
#include "include/cef_application_mac.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_runnable.h"
#include "cefclient/cefclient.h"
#include "cefclient/client_handler.h"
#include "cefclient/client_switches.h"
#include "cefclient/cookie_handler.h"
#include "chromium_loader/signal_restore_posix.h"

namespace {

char* szWorkingDir;  // The current working directory
bool message_loop = false;
bool handling_send_event = false;
CefWindowHandle mainBrowserHandle = NULL;

} // namespace

// Send event handling is copied from javachromiumembedded util_mac.mm.
// Java provides an NSApplicationAWT implementation that we can't access or
// override directly. Therefore add the necessary CrAppControlProtocol
// functionality to NSApplication using categories and swizzling.
@interface NSApplication (JavaCefApplication)

- (BOOL)isHandlingSendEvent;
- (void)setHandlingSendEvent:(BOOL)handlingSendEvent;
- (void)_swizzled_sendEvent:(NSEvent*)event;

@end

@implementation NSApplication (JavaCefApplication)

// This selector is called very early during the application initialization.
+ (void)load {
  // Swap NSApplication::sendEvent with _swizzled_sendEvent.
  Method original = class_getInstanceMethod(self, @selector(sendEvent));
  Method swizzled =
  class_getInstanceMethod(self, @selector(_swizzled_sendEvent));
  method_exchangeImplementations(original, swizzled);
}

- (BOOL)isHandlingSendEvent {
  return handling_send_event;
}

- (void)setHandlingSendEvent:(BOOL)handlingSendEvent {
  handling_send_event = handlingSendEvent;
}

- (void)_swizzled_sendEvent:(NSEvent*)event {
  CefScopedSendingEvent sendingEventScoper;
  // Calls NSApplication::sendEvent due to the swizzling.
  [self _swizzled_sendEvent:event];
}

@end

// The linker has -fvisibility=hidden by default, so we need to explictly
// export the jni functions.
#undef JNIEXPORT
#define JNIEXPORT __attribute ((visibility("default")))

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1init
  (JNIEnv *env, jobject jobj, jlong hwnd, jstring url, jobject chromiumset)
{
  // Make a simple argument.
  const int argc = 1;
  char** argv = (char**)malloc(argc * sizeof(*argv));
  argv[0] = strdup("java");

  CefMainArgs main_args(argc, argv);
  CefRefPtr<ClientApp> app(new ClientApp);

  // Retrieve the current working directory.
  szWorkingDir = getenv("JAVACEF_PATH");
  if (!szWorkingDir)
    szWorkingDir = (char*)calloc(1, sizeof(char));

  // Parse command line arguments. The passed in values are ignored on Windows.
  AppInitCommandLine(argc, argv);

  CefSettings settings;

  // Populate the settings based on command line arguments.
  AppGetSettings(settings);

  settings.multi_threaded_message_loop = message_loop;
  settings.log_severity = LOGSEVERITY_DISABLE;

  CefString path = CefString(szWorkingDir);

#ifndef __LP64__
  CefString(&settings.browser_subprocess_path) = path.ToString() + "/cef_runtime/mac32/cefclient.app/Contents/Frameworks/cefclient Helper.app/Contents/MacOS/cefclient Helper";
  CefString(&settings.resources_dir_path) = path.ToString() + "/cef_runtime/mac32/cefclient.app/Contents/Frameworks/Chromium Embedded Framework.framework/Resources";
#else
  CefString(&settings.browser_subprocess_path) = path.ToString() + "/cef_runtime/mac64/cefclient.app/Contents/Frameworks/cefclient Helper.app/Contents/MacOS/cefclient Helper";

  // Need to set the path to find devtools resources.
  CefString(&settings.resources_dir_path) = path.ToString() + "/cef_runtime/mac64/cefclient.app/Contents/Frameworks/Chromium Embedded Framework.framework/Resources";
#endif

  BackupSignalHandlers();

  // Initialize CEF.
  CefInitialize(main_args, settings, app.get(), NULL);

  RestoreSignalHandlers();

  NSView* view = (NSView*)hwnd;
  NSThread* thread = [NSThread currentThread];
  if ([thread isMainThread])
    printf("[javacef] main thread\n");

  const char* chr = env->GetStringUTFChars(url, 0);
  CefString wc = chr;

  CefRefPtr<ClientHandler> gh = InitBrowser(view, wc);
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
/*
  if (!message_loop) {
    CefRunMessageLoop();
    CefShutdown();
  }
*/
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1new
  (JNIEnv *env, jobject jobj, jlong hwnd, jint id, jstring url, jobject chromiumset)
{
  const char* chr = env->GetStringUTFChars(url, 0);
  CefString wc = chr;
  CefRefPtr<ClientHandler> gh = NewBrowser((CefWindowHandle)hwnd, wc);
  gh->id = id;
  env->ReleaseStringUTFChars(url, chr);
  send_handler(env, jobj, (jlong)(void*)gh);
  get_browser_settings(env, chromiumset, gh->csettings);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1message_1loop
  (JNIEnv *env, jobject jobj)
{
  CefDoMessageLoopWork();
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
    g_handler_local->id = -1;
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
    g_handler_local->id = -1;
    mainBrowserHandle = g_handler_local->GetBrowser()->GetHost()->GetWindowHandle();
  }
  CefShutdown();
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
  NSView* view = (CefWindowHandle)hwnd;

  if (g_handler_local.get() && g_handler_local->GetBrowser() &&
      g_handler_local->GetBrowser()->GetHost()->GetWindowHandle()) {
    NSArray* subviews = [view subviews];
    if ([subviews count] > 0) {
      NSView* browser_view = [subviews objectAtIndex:0];
      NSRect bound = [view bounds];
      [browser_view setFrame:bound];
    }
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
  // do nothing
}
