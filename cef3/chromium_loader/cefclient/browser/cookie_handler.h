// Copyright (c) 2012 javacef Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef COOKIE_HANDLER_H_
#define COOKIE_HANDLER_H_
#pragma once

#include "include/cef_cookie.h"

class CookieHandler : public CefCookieVisitor {
 public:
  CookieHandler();
  CookieHandler(bool delete_all_cookies);
  virtual ~CookieHandler();

  virtual bool Visit(const CefCookie& cookie, int count, int total, bool& deleteCookie) OVERRIDE;

 protected:
  IMPLEMENT_REFCOUNTING(CookieHandler);
 private:
  bool delete_all;
};

#endif // COOKIE_HANDLER_H_
