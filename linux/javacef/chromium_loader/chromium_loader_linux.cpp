
#include "org_embedded_browser_Chromium.h"
#include "org_embedded_browser_DownloadWindow.h"

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
#include "jni_tools.h"

void TerminationSignalHandler(int signal) {
  CefQuitMessageLoop();
  printf("quit loop\n");
}

// The linker has -fvisibility=hidden by default, so we need to explictly
// export the jni functions.
#undef JNIEXPORT
#define JNIEXPORT __attribute ((visibility("default")))

JNIEXPORT jlong JNICALL Java_org_embedded_browser_Chromium_browser_1init
  (JNIEnv *env, jobject jobj, jlong hwnd, jstring url, jobject chromiumset)
{
  // Make a simple argument.
  const int argc = 1;
  char** argv = (char**)malloc(argc * sizeof(*argv));
  argv[0] = strdup("java");

  CefMainArgs main_args(argc, argv);
  CefRefPtr<ClientApp> app(new ClientApp);
  CefSettings settings;

  // Execute the secondary process, if any.
  int exit_code = CefExecuteProcess(main_args, app.get());
  if (exit_code >= 0)
    return exit_code;

  // Parse command line arguments. The passed in values are ignored on Windows.
  AppInitCommandLine(argc, argv);

  // Populate the settings based on command line arguments.
  AppGetSettings(settings);

  // Initialize CEF.
  CefInitialize(main_args, settings, app.get());

  GtkWidget* canvas = (GtkWidget*)hwnd;
  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  gtk_fixed_put(GTK_FIXED(canvas), vbox, 0, 0);

  //GtkWidget* scroll_window = gtk_scrolled_window_new(NULL, NULL);
  //GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  //gtk_container_add(GTK_CONTAINER(scroll_window), vbox);
  //gtk_fixed_put(GTK_FIXED(canvas), scroll_window, 0, 0);

  set_java_env(env, jobj);

  const char* chr = env->GetStringUTFChars(url, 0);
  CefString wc = chr;

  CefRefPtr<ClientHandler> gh = InitBrowser(vbox, wc);
  gh->id = 1;

  env->ReleaseStringUTFChars(url, chr);

  // NOTE: This function (browser_init) should only be called ONCE in one process,
  // which means to call CefInitialize in the first broswer creation, and call 
  // CefShutdown in the exit of the process. Repeated calls to this function will 
  // NOT work, because the web page will not render. Most probably a bug in cef.
  send_handler(env, jobj, (jlong)(void*)gh);

  // Have to be here and use own jnienv to avoid errors.
  get_browser_settings(env, chromiumset, gh->csettings);

  //signal(SIGINT, TerminationSignalHandler);
  //signal(SIGTERM, TerminationSignalHandler);

  //CefRunMessageLoop();
  //CefShutdown();
  return (jlong)vbox;
}

JNIEXPORT jlong JNICALL Java_org_embedded_browser_Chromium_browser_1new
  (JNIEnv *env, jobject jobj, jlong hwnd, jint id, jstring url, jobject chromiumset)
{
  const char* chr = env->GetStringUTFChars(url, 0);
  CefString wc = chr;

  GtkWidget* canvas = (GtkWidget*)hwnd;
  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  gtk_fixed_put(GTK_FIXED(canvas), vbox, 0, 0);
  set_java_env(env, jobj);

  CefRefPtr<ClientHandler> gh = NewBrowser(vbox, wc);
  gh->id = id;
  env->ReleaseStringUTFChars(url, chr);
  send_handler(env, jobj, (jlong)(void*)gh);
  get_browser_settings(env, chromiumset, gh->csettings);//*/
  printf("%d\n", id);
  fflush(stdout);

  return (jlong)vbox;
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1message_1loop
  (JNIEnv *env, jobject jobj)
{
  CefDoMessageLoopWork();
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1close
  (JNIEnv *env, jobject jobj, jlong gh)
{
  CefRefPtr<ClientHandler> g_handler_local;
  g_handler_local = (ClientHandler*)gh;
  printf("gl0: %lu\n", (jlong)(void*)g_handler_local);
  printf("gl11: %d\n", g_handler_local->id);
  /*if (g_handler_local.get()) {
    CefRefPtr<CefBrowserHost> browserhost = g_handler_local->GetBrowser()->GetHost();
    if (browserhost.get()) {
      // Let the browser host know we are about to destroy it.
      //browserhost->ParentWindowWillClose();
	    browserhost->CloseBrowser();
  printf("ok: %lu\n", (jlong)(void*)browserhost.get());
    }
  }
  printf("gl1: %lu\n", (jlong)(void*)gh);
  asm("push %%rax":);
  asm("mov %0,%%rax"::"m"(gh):);
  asm("mov %%rax,%0":"=m"(g_handler_local):);
  asm("pop %%rax":);
  printf("gla: %lu\n", (jlong)(void*)g_handler_local);
  fflush(stdout);//*/
  //CloseBrowser(gh);
  //printf("close\n");
  //g_handler_local.get()->Release();
  printf("gl2: %lu\n", (jlong)(void*)g_handler_local->GetBrowser()->GetHost());
  fflush(stdout);
  g_handler_local->GetBrowser()->GetHost()->CloseBrowser();
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1shutdown
  (JNIEnv *env, jobject jobj, jlong gh)
{
  //asm("int $3");
  //ShutdownBrowser(gh);
  printf("sd\n");
  fflush(stdout);
  CefShutdown();
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1clean_1cookies
  (JNIEnv *env, jobject jobj)
{
  //CleanCookies();
  printf("cc\n");
  fflush(stdout);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1setUrl
  (JNIEnv *env, jobject jobj, jlong gh, jstring url)
{
/*  const char* chr = env->GetStringUTFChars(url, 0);
  CefString wc = chr;
  if (chr)
    setUrl(gh, wc);
  env->ReleaseStringUTFChars(url, chr);*/
  printf("su\n");
  fflush(stdout);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1resized
  (JNIEnv *env, jobject jobj, jlong chandle, jlong vhandle)
{
  //resized(gh, hwnd);

  /*GtkWidget* vbox = (GtkWidget*)hwnd;

  printf("rs: %lu, %lu\n", hwnd, (jlong)vbox->window);
  fflush(stdout);

  if (vbox->window)
    gtk_widget_show_all(GTK_WIDGET(vbox));*/
  GtkWidget* canvas = (GtkWidget*)chandle;
  GtkWidget* vbox = (GtkWidget*)vhandle;

  GtkAllocation* allocation = new GtkAllocation();
  gtk_widget_get_allocation(canvas, allocation);

  printf("rs: %lu, %lu, %d, %d\n", chandle, vhandle, allocation->width, allocation->height);
  fflush(stdout);

  gtk_widget_show_all(GTK_WIDGET(canvas));
  gtk_widget_set_size_request(vbox, allocation->width, allocation->height);

  delete allocation;//*/
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1back
  (JNIEnv *env, jobject jobj, jlong gh)
{
  //back(gh);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1forward
  (JNIEnv *env, jobject jobj, jlong gh)
{
  //forward(gh);
}

JNIEXPORT void JNICALL Java_org_embedded_browser_Chromium_browser_1reload
  (JNIEnv *env, jobject jobj, jlong gh)
{
  //reload(gh);
  printf("rl\n");
  fflush(stdout);
}

JNIEXPORT jlong JNICALL Java_org_embedded_browser_DownloadWindow_getReceivedN
  (JNIEnv *env, jobject jobj, jint dhint) // TODO: fix long
{
/*  CefRefPtr<CefDownloadHandlerW> d_handler;
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
