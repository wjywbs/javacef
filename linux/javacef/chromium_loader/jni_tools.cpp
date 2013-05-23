#include "jni_tools.h"
#include <stdlib.h>
#include <sys/syscall.h>

JNIEnv* envs = NULL;
jobject jobjs = NULL;
JavaVM *jvm = NULL;
jclass cls;

bool in_java_runtime() {
  return envs != NULL;
}

void set_java_env(JNIEnv* env, jobject jobj)
{
  envs = env;
  jobjs = jobj;
  cls = env->FindClass("org/embedded/browser/Chromium");
}

void send_handler(JNIEnv* env, jobject jobj, jlong gh)
{
  jclass cls = env->FindClass("org/embedded/browser/Chromium");
  jmethodID mid = env->GetMethodID(cls, "loadfinish", "(J)V");
  env->CallObjectMethod(jobj, mid, gh);
}

void set_title(const char* title, int id)
{
  jclass cls = envs->FindClass("org/embedded/browser/Chromium");
  jmethodID mid = envs->GetMethodID(cls, "title_change", "(Ljava/lang/String;I)V"); // TODO: fix java side!!!
  envs->CallObjectMethod(jobjs, mid, stringtojstring(envs, title), id);
}

jobject get_download_window(const char* fn, long long size, const char* mime)
{
  jclass cls = envs->FindClass("org/embedded/browser/DownloadWindow");
  jmethodID mid = envs->GetMethodID(cls, "<init>", "(Ljava/lang/String;JLjava/lang/String;)V"); // TODO: fix java side!!!
  return envs->NewObject(cls, mid, stringtojstring(envs, fn), size, stringtojstring(envs, mime));
}

std::string get_download_path_init(jobject dw)
{
  jclass cls = envs->FindClass("org/embedded/browser/DownloadWindow");
  jmethodID mid = envs->GetMethodID(cls, "getPathAndInit", "()Ljava/lang/String;");
  jstring jpath = (jstring)envs->CallObjectMethod(dw, mid);
  const char* chr = envs->GetStringUTFChars(jpath, 0);
  std::string path(chr);
  envs->ReleaseStringUTFChars(jpath, chr);
  return path;
}

void send_download_handler(jobject dw, int dh)
{
  jclass cls = envs->FindClass("org/embedded/browser/DownloadWindow");
  jmethodID mid = envs->GetMethodID(cls, "set_dhptr", "(I)V"); // TODO: fix java side!!!
  envs->CallObjectMethod(dw, mid, dh);
}

void send_download_status(jobject dw, int ds)
{
  jclass cls = envs->FindClass("org/embedded/browser/DownloadWindow");
  jmethodID msid = envs->GetMethodID(cls, "set_status", "(I)V"); // TODO: fix java side!!!
  envs->CallObjectMethod(dw, msid, ds);
}

void new_tab(int id, std::string url)
{
  jclass cls = envs->FindClass("org/embedded/browser/Chromium");
  jmethodID mid = envs->GetMethodID(cls, "new_window", "(ILjava/lang/String;)V");//(I)Lorg/embedded/browser/Chromium;
  envs->CallObjectMethod(jobjs, mid, id, stringtojstring(envs, url.c_str()));
  //jfieldID fidc = envs->GetFieldID(cls, "chptr", "I");
  //envs->SetIntField(bobj, fidc, gh);
  //jfieldID fidh = envs->GetFieldID(cls, "hwnd", "I");
  //hwnd = envs->GetIntField(bobj, fidh);
  //return bobj;get_chromium
}

void close_tab(int id)
{
  jclass cls = envs->FindClass("org/embedded/browser/Chromium");
  jmethodID mid = envs->GetMethodID(cls, "close_window", "(I)V"); // TODO: fix java side!!!
  envs->CallObjectMethod(jobjs, mid, id);
}

void send_load(int id, bool loading)
{
  pid_t tid = syscall(SYS_gettid);
  printf("cpploadcls: %d\n", tid);
  jclass cls = envs->FindClass("org/embedded/browser/Chromium");
  printf("cpploadcls: %lu\n", (jlong)(void*)cls);
  jmethodID mid = envs->GetMethodID(cls, "load_change", "(IZ)V"); // TODO: fix java side!!!
  printf("cpploadcls: %lu\n", (jlong)(void*)mid);
  fflush(stdout);
  //env->CallObjectMethod(jobjs, mid, id, loading);
  envs->CallStaticVoidMethod(cls, mid, (jvalue*)(jlong)id, loading);
  printf("cpploadcls: %lu\n", (jlong)(void*)jobjs);
  fflush(stdout);
}

void send_navstate(int id, bool canGoBack, bool canGoForward)
{
  jclass cls = envs->FindClass("org/embedded/browser/Chromium");
  jmethodID mid = envs->GetMethodID(cls, "navstate_change", "(IZZ)V"); // TODO: fix java side!!!
  envs->CallObjectMethod(jobjs, mid, id, canGoBack, canGoForward);
}

void get_browser_settings(JNIEnv* env, jobject jcset, ChromiumSettings& cset)
{
  jclass cls = env->FindClass("org/embedded/browser/ChromeSettings");
  jfieldID allow_right_button = env->GetFieldID(cls, "allow_right_button", "Z");
  cset.allow_right_button = (bool)env->GetBooleanField(jcset, allow_right_button);

  jfieldID keyid = env->GetFieldID(cls, "keys", "[Ljava/lang/String;");
  jobjectArray keys = (jobjectArray)env->GetObjectField(jcset, keyid);
  jint klen = env->GetArrayLength(keys);
  jfieldID valueid = env->GetFieldID(cls, "values", "[Ljava/lang/String;");
  jobjectArray values = (jobjectArray)env->GetObjectField(jcset, valueid);
  //jint vlen = env->GetArrayLength(values);
  for (int i = 0; i < klen; i++)
  {
    jstring kstr = (jstring)env->GetObjectArrayElement(keys, (jsize)i);
    jstring vstr = (jstring)env->GetObjectArrayElement(values, (jsize)i);
    const char* kchr = env->GetStringUTFChars(kstr, 0);
    const char* vchr = env->GetStringUTFChars(vstr, 0);
    cset.cookies[std::string(kchr)] = std::string(vchr);
    env->ReleaseStringUTFChars(kstr, kchr);
    env->ReleaseStringUTFChars(vstr, vchr);
  }
}

jstring stringtojstring(JNIEnv* env, const char* pat)
{
jclass strClass = env->FindClass("Ljava/lang/String;");
jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
jbyteArray bytes = env->NewByteArray(strlen(pat));
env->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
jstring encoding = env->NewStringUTF("utf-8");
return (jstring)env->NewObject(strClass, ctorID, bytes, encoding);
}
