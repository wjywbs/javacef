// Copyright (c) 2014 javacef Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "chromium_loader/signal_restore_posix.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <map>

// arraysize borrowed from base/macros.h
template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))

namespace {

const int signals_to_restore[] =
      {SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGSEGV,
       SIGALRM, SIGTERM, SIGCHLD, SIGBUS, SIGTRAP, SIGPIPE};

std::map<int, struct sigaction> signal_handlers;

} // namespace

void BackupSignalHandlers() {
  struct sigaction sigact;
  for (unsigned i = 0; i < arraysize(signals_to_restore); i++) {
    memset(&sigact, 0, sizeof(sigact));
    sigaction(signals_to_restore[i], NULL, &sigact);
    signal_handlers[signals_to_restore[i]] = sigact;
  }
}

void RestoreSignalHandlers() {
  for (unsigned i = 0; i < arraysize(signals_to_restore); i++) {
    sigaction(signals_to_restore[i],
              &signal_handlers[signals_to_restore[i]],
              NULL);
  }
}
