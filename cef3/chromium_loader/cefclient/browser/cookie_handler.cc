// Copyright (c) 2012 javacef Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cookie_handler.h"

CookieHandler::CookieHandler() { delete_all = false; }

CookieHandler::CookieHandler(bool delete_all_cookies) {
  delete_all = delete_all_cookies;
}

CookieHandler::~CookieHandler() { }

bool CookieHandler::Visit(const CefCookie& cookie, int count, 
                          int total, bool& deleteCookie) {
  deleteCookie = delete_all;
  return true;
}
