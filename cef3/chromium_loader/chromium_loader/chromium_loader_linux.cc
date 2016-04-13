
#include "org_embedded_browser_Chromium.h"
#include "org_embedded_browser_DownloadWindow.h"
#include "jni_tools.h"

#include <stdlib.h>
#include <sstream>
#include <string>
#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_runnable.h"
#include "cefclient/string_util.h"
#include "cefclient/cefclient.h"
#include "cefclient/client_handler.h"
#include "cefclient/cookie_handler.h"
#include "chromium_loader/signal_restore_posix.h"

namespace {

char* szWorkingDir;  // The current working directory
bool message_loop = false;

} // namespace

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
  CefString(&settings.browser_subprocess_path) = path.ToString() + "/cef_runtime/linux32/cefclient";
  CefString(&settings.resources_dir_path) = path.ToString() + "/cef_runtime/linux32";
  CefString(&settings.locales_dir_path) = path.ToString() + "/cef_runtime/linux32/locales";
#else
  CefString(&settings.browser_subprocess_path) = path.ToString() + "/cef_runtime/linux64/cefclient";
  CefString(&settings.resources_dir_path) = path.ToString() + "/cef_runtime/linux64";
  CefString(&settings.locales_dir_path) = path.ToString() + "/cef_runtime/linux64/locales";
#endif

  BackupSignalHandlers();

  // Initialize CEF.
  CefInitialize(main_args, settings, app.get(), NULL);

  RestoreSignalHandlers();

  GtkWidget* canvas = (GtkWidget*)hwnd;
  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  gtk_fixed_put(GTK_FIXED(canvas), vbox, 0, 0);

  const char* chr = env->GetStringUTFChars(url, 0);
  CefString wc = chr;

  CefRefPtr<ClientHandler> gh = InitBrowser(vbox, wc);
  gh->id = 1;
  gh->vbox = vbox;

  env->ReleaseStringUTFChars(url, chr);

  // NOTE: This function (browser_init) should only be called ONCE in one process,
  // which means to call CefInitialize in the first broswer creation, and call 
  // CefShutdown in the exit of the process. Repeated calls to this function will 
  // NOT work, because the web page will not render. Most probably a bug in cef.
  set_jvm(env, jobj);
  send_handler(env, jobj, (jlong)(void*)gh);

  // Have to be here and use own jnienv to avoid errors.
  get_browser_settings(env, chromiumset, gh->csettings);

  //CefRunMessageLoop();
  //CefShutdown();
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1new
  (JNIEnv *env, jobject jobj, jlong hwnd, jint id, jstring url, jobject chromiumset)
{
  const char* chr = env->GetStringUTFChars(url, 0);
  CefString wc = chr;

  GtkWidget* canvas = (GtkWidget*)hwnd;
  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  gtk_fixed_put(GTK_FIXED(canvas), vbox, 0, 0);

  CefRefPtr<ClientHandler> gh = NewBrowser(vbox, wc);
  gh->id = id;
  gh->vbox = vbox;
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

  if (g_handler_local.get() && g_handler_local->GetBrowser()) {
    g_handler_local->GetBrowser()->GetHost()->CloseBrowser(false);
    g_handler_local->id = -1;
  }
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
  }

  cleanup_jvm(env);
  CefShutdown();
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1clean_1cookies
  (JNIEnv *env, jobject jobj)
{
  CefRefPtr<CefCookieManager> cookieManager = CefCookieManager::GetGlobalManager();
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
  GtkWidget* canvas = (GtkWidget*)hwnd;

  GtkAllocation* allocation = new GtkAllocation();
  gtk_widget_get_allocation(canvas, allocation);

  gtk_widget_show_all(GTK_WIDGET(canvas));
  gtk_widget_set_size_request(g_handler_local->vbox,
                              allocation->width,
                              allocation->height);

  delete allocation;
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
  (JNIEnv *env, jobject jobj, jint dhint) // TODO: fix long
{
  return 0;
}

// Global functions

std::string AppGetWorkingDirectory() {
  return szWorkingDir;
}

void AppQuitMessageLoop() {
  // Do nothing.
}

