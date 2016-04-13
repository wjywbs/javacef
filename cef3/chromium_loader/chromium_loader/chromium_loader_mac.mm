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
#include "cefclient/browser/client_app_browser.h"
#include "cefclient/browser/client_handler.h"
#include "cefclient/browser/cookie_handler.h"
#include "cefclient/browser/main_message_loop_std.h"
#include "cefclient/common/client_switches.h"
#include "chromium_loader/browser_creator.h"
#include "chromium_loader/signal_restore_posix.h"

using ClientHandler = client::ClientHandler;

namespace {

bool use_message_loop = false;
bool handling_send_event = false;
CefString resources_dir_path;
client::MainMessageLoopStd* message_loop = NULL;

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

@interface NSBundle (JavaCefBundle)

- (NSString *)_swizzled_pathForResource:(NSString *)name
                                 ofType:(NSString *)extension;

- (NSString *)_swizzled_pathForResource:(NSString *)name
                                 ofType:(NSString *)extension
                            inDirectory:(NSString *)subpath
                        forLocalization:(NSString *)localizationName;

@end

@implementation NSBundle (JavaCefBundle)

+ (void)load {
  // Swap NSApplication::pathForResource with _swizzled_pathForResource.
  Method original = class_getInstanceMethod(self, @selector(pathForResource:ofType:));
  Method swizzled = class_getInstanceMethod(self, @selector(_swizzled_pathForResource:ofType:));
  method_exchangeImplementations(original, swizzled);

  original = class_getInstanceMethod(self, @selector(pathForResource:ofType:inDirectory:forLocalization:));
  swizzled = class_getInstanceMethod(self, @selector(_swizzled_pathForResource:ofType:inDirectory:forLocalization:));
  method_exchangeImplementations(original, swizzled);
}

- (NSString *)_swizzled_pathForResource:(NSString *)name
                                 ofType:(NSString *)extension {
  // Try to use the original resolver.
  NSString* original = [self _swizzled_pathForResource:name ofType:extension];
  if (original)
    return original;

  // Load resources from resources_dir_path.
  std::string ext = ".";
  ext = extension ? ext + [extension UTF8String] : "";

  const std::string resources_dir = (const std::string)resources_dir_path;
  std::string path = resources_dir + "/" + [name UTF8String] + ext;
  return [NSString stringWithUTF8String:path.c_str()];
}

- (NSString *)_swizzled_pathForResource:(NSString *)name
                                 ofType:(NSString *)extension
                            inDirectory:(NSString *)subpath
                        forLocalization:(NSString *)localizationName {
  std::string ext = ".";
  ext = extension ? ext + [extension UTF8String] : "";

  const std::string resources_dir = (const std::string)resources_dir_path;
  std::string path = resources_dir + "/" + [localizationName UTF8String] + ".lproj/" + [name UTF8String] + ext;
  return [NSString stringWithUTF8String:path.c_str()];
}

@end

void GetBrowserWindowInfo(CefWindowInfo& info, CefWindowHandle handle) {
  // The size may be (0,0)
  NSSize size = handle.frame.size;

  // Initialize window info to the defaults for a child window
  info.SetAsChild(handle, 0, 0, size.width, size.height);
}

// The linker has -fvisibility=hidden by default, so we need to explictly
// export the jni functions.
#undef JNIEXPORT
#define JNIEXPORT __attribute ((visibility("default")))

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1init
  (JNIEnv *env, jobject jobj, jlong hwnd, jstring url, jobject chromiumset)
{
  CefMainArgs main_args(0, NULL);

  CefRefPtr<CefApp> app(new client::ClientAppBrowser);
  CefSettings settings;

  settings.multi_threaded_message_loop = use_message_loop;
  settings.log_severity = LOGSEVERITY_DISABLE;
  //settings.no_sandbox = true;
  //settings.background_color = CefColorSetARGB(255, 255, 255, 255);

  // Retrieve the current working directory.
  char* szWorkingDir = getenv("JAVACEF_PATH");
  if (!szWorkingDir)
    szWorkingDir = (char*)calloc(1, sizeof(char));

  CefString path = CefString(szWorkingDir);
  CefString(&settings.browser_subprocess_path) = path.ToString() + "/cef_runtime/mac64/cefclient.app/Contents/Frameworks/cefclient Helper.app/Contents/MacOS/cefclient Helper";

  // Need to set the path to find devtools resources.
  resources_dir_path = path.ToString() + "/cef_runtime/mac64/cefclient.app/Contents/Frameworks/Chromium Embedded Framework.framework/Resources";
  CefString(&settings.resources_dir_path) = resources_dir_path;

  message_loop = new client::MainMessageLoopStd();

  BackupSignalHandlers();

  if (!CefInitialize(main_args, settings, app, NULL)) {
    fprintf(stderr, "Failed to initialize CEF.\n");
    return;
  }

  RestoreSignalHandlers();

  NSView* view = (NSView*)hwnd;
  NSThread* thread = [NSThread currentThread];
  if ([thread isMainThread])
    printf("[javacef] main thread\n");

  const char* chr = env->GetStringUTFChars(url, 0);
  CefString wc = chr;

  CefRefPtr<ClientHandler> gh = NewBrowser(view, wc);
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
  if (!use_message_loop) {
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
    g_handler_local->GetBrowser()->GetHost()->CloseBrowser(true);
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
    g_handler_local->GetBrowser()->GetHost()->CloseBrowser(true);
    g_handler_local->id = -1;
  }
  cleanup_jvm(env);
  CefShutdown();

  if (message_loop) {
    delete message_loop;
    message_loop = NULL;
  }
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1clean_1cookies
  (JNIEnv *env, jobject jobj)
{
  CefRefPtr<CefCookieManager> cookieManager = CefCookieManager::GetGlobalManager(NULL);
  CefRefPtr<CefCookieVisitor> cookieHandler = new CookieHandler(true);
  cookieManager->VisitAllCookies(cookieHandler);
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
      NSSize size = bound.size;
      //printf("subviews %lu w %f h %f\n", [subviews count], size.width, size.height);
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
