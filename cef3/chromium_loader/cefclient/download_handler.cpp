// Copyright (c) 2012 javacef Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"
#include "download_handler.h"
#include <stdio.h>
#include <sstream>
#include <vector>
#include "include/cef_download_handler.h"
#include "include/cef_runnable.h"
#include "util.h"
#include "jni_tools.h"

#if defined(OS_WIN)
#include <windows.h>  // NOLINT(build/include_order)
#include <shlobj.h>  // NOLINT(build/include_order)
#include <shlwapi.h>  // NOLINT(build/include_order)
#endif  // OS_WIN

// Implementation of the CefDownloadHandler interface.
class ClientDownloadHandler : public CefDownloadHandlerW {
 public:
  ClientDownloadHandler(CefRefPtr<DownloadListener> listener,
                        const CefString& fileName)
    : listener_(listener), filename_(fileName), file_(NULL) {
  }

  ~ClientDownloadHandler() {
    //ASSERT(pending_data_.empty());
    //ASSERT(file_ == NULL);

    if (!pending_data_.empty()) {
      // Delete remaining pending data.
      std::vector<std::vector<char>*>::iterator it = pending_data_.begin();
      for (; it != pending_data_.end(); ++it)
        delete (*it);
    }

    if (file_) {
      // Close the dangling file pointer on the FILE thread.
      CefPostTask(TID_FILE,
          NewCefRunnableFunction(&ClientDownloadHandler::CloseDanglingFile,
                                 file_));

      // Notify the listener that the download failed.
      send_download_status(jdw, S_ERROR);
    }
  }

#define flush_size 1000000 //about 1MB

  // --------------------------------------------------
  // The following methods are called on the UI thread.
  // --------------------------------------------------

  void Initialize() {
    received_size = 0;
    buffer_size = 0;
    bcontinue = true;
    // Open the file on the FILE thread.
    CefPostTask(TID_FILE,
        NewCefRunnableMethod(this, &ClientDownloadHandler::OnOpen));
  }

  // A portion of the file contents have been received. This method will be
  // called multiple times until the download is complete. Return |true| to
  // continue receiving data and |false| to cancel.
  virtual bool ReceivedData(void* data, int data_size) {
    REQUIRE_UI_THREAD();

    if (data_size == 0)
      return true;
    received_size += data_size;
    buffer_size += data_size;

    // Create a new vector for the data.
    std::vector<char>* buffer = new std::vector<char>(data_size);
    memcpy(&(*buffer)[0], data, data_size);

    // Add the new data vector to the pending data queue.
    {
      AutoLock lock_scope(this);
      pending_data_.push_back(buffer);
    }

    // Write data to file on the FILE thread.
    if (buffer_size > flush_size) {
      buffer_size = 0;
      CefPostTask(TID_FILE,
          NewCefRunnableMethod(this, &ClientDownloadHandler::OnReceivedData));
    }
    return bcontinue;
  }

  // The download is complete.
  virtual void Complete() {
    REQUIRE_UI_THREAD();

    // Flush and close the file on the FILE thread.
    CefPostTask(TID_FILE,
        NewCefRunnableMethod(this, &ClientDownloadHandler::OnComplete));
  }

  virtual void SendComplete() {
    REQUIRE_UI_THREAD();

    // Have to put it here. JNI calls require same thread.
    send_download_status(jdw, S_DOWNLOADED);
  }

  // ----------------------------------------------------
  // The following methods are called on the FILE thread.
  // ----------------------------------------------------

  void OnOpen() {
    REQUIRE_FILE_THREAD();

    if (file_)
      return;

#if defined(OS_WIN)
      std::wstring fileNameStr = filename_;

      file_ = _wfopen(fileNameStr.c_str(), L"wb");
      ASSERT(file_ != NULL);
#else
    // TODO(port): Implement this.
    ASSERT(false);  // Not implemented
#endif
  }

  void OnComplete() {
    REQUIRE_FILE_THREAD();

    if (!file_)
      return;

    // Make sure any pending data is written.
    OnReceivedData();

    fclose(file_);
    file_ = NULL;

    // Notify the listener that the download completed.
    CefPostTask(TID_UI,
        NewCefRunnableMethod(this, &ClientDownloadHandler::SendComplete));
  }

  void OnReceivedData() {
    REQUIRE_FILE_THREAD();

    std::vector<std::vector<char>*> data;

    // Remove all data from the pending data queue.
    {
      AutoLock lock_scope(this);
      if (!pending_data_.empty()) {
        data = pending_data_;
        pending_data_.clear();
      }
    }

    if (data.empty())
      return;

    // Write all pending data to file.
    std::vector<std::vector<char>*>::iterator it = data.begin();
    for (; it != data.end(); ++it) {
      std::vector<char>* buffer = *it;
      if (file_)
        fwrite(&(*buffer)[0], buffer->size(), 1, file_);
      delete buffer;
    }
    data.clear();
  }

  static void CloseDanglingFile(FILE *file) {
    fclose(file);
  }

 private:
  CefRefPtr<DownloadListener> listener_;
  CefString filename_;
  FILE* file_;
  std::vector<std::vector<char>*> pending_data_;
  int64 buffer_size;

  IMPLEMENT_REFCOUNTING(ClientDownloadHandler);
  IMPLEMENT_LOCKING(ClientDownloadHandler);
};

CefRefPtr<CefDownloadHandler> CreateDownloadHandler(
    CefRefPtr<DownloadListener> listener, const CefString& fileName, jobject dw) {
  CefRefPtr<ClientDownloadHandler> handler =
      new ClientDownloadHandler(listener, fileName);
  handler->jdw = dw;
  handler->Initialize();
  return handler.get();
}
