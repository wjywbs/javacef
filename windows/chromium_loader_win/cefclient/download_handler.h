// Copyright (c) 2012 javacef Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFCLIENT_DOWNLOAD_HANDLER_H_
#define CEF_TESTS_CEFCLIENT_DOWNLOAD_HANDLER_H_
#pragma once

#include "../include/cef_base.h"
#include "../include/cef_download_handler.h"
#include <jni.h>

class CefDownloadHandlerW : public CefDownloadHandler {
 public:
  int64 received_size;
  bool bcontinue;
  jobject jdw;
};

// Implement this interface to receive download notifications.
class DownloadListener : public virtual CefBase {
 public:
  // Called when the download is complete.
  virtual void NotifyDownloadComplete(const CefString& fileName) =0;

  // Called if the download fails.
  virtual void NotifyDownloadError(const CefString& fileName) =0;
};

// Create a new download handler to manage download of a single file.
CefRefPtr<CefDownloadHandler> CreateDownloadHandler(
    CefRefPtr<DownloadListener> listener, const CefString& fileName, jobject dw);

#endif  // CEF_TESTS_CEFCLIENT_DOWNLOAD_HANDLER_H_
