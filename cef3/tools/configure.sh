#!/bin/bash

if [ -z "$1" ]; then
  echo "Usage: ./configure.sh mac64|linux64|linux32"
  exit 1
fi

PLATFORM=$1

TOOL_DIR="$( cd "$( dirname "$0" )" && pwd )"
. "$TOOL_DIR/common.sh"

if [ ! -d "$GYP_DIR" ]; then
  echo "Checking out gyp into tools/gyp..."
  git clone https://chromium.googlesource.com/external/gyp "$GYP_DIR"
fi

if [ -z "$GYP_GENERATORS" ]; then
  if [ -z `which ninja` ]; then
    export GYP_GENERATORS=make
  else
    export GYP_GENERATORS=ninja
  fi
fi

if [ "$PLATFORM" == "mac64" ]; then
  checkJavaHomeMac 64
  checkJavaIncludeMac 64
  checkCefPrebuiltMac
  printGYPGenerator

  "$TOOL_DIR/gyp_chromium" --toplevel-dir="$PROJECT_DIR" --depth="$TOOL_DIR" \
      -Djava_include_path="$JAVA_INCLUDE_PATH" -Dcef_prebuilt_path="$APP_PATH" \
      -Dmac_deployment_target=10.7 -Dmac_sdk_min=10.10 "$PROJECT_DIR/javacef.gyp"
fi

if [ "$PLATFORM" == "mac32" ]; then
  echo "mac32 is no longer supported in cef 2526. Try 3.1750 branch instead."
  exit 1
fi

if [ "$PLATFORM" == "linux64" ]; then
  checkJavaLinux
  checkCefPrebuiltLinux
  printGYPGenerator

  "$TOOL_DIR/gyp_chromium" --toplevel-dir="$PROJECT_DIR" --depth="$TOOL_DIR" \
      -Djava_include_path="$JAVA_INCLUDE_PATH" -Dcef_prebuilt_path="$APP_PATH" \
      "$PROJECT_DIR/javacef.gyp"
fi

if [ "$PLATFORM" == "linux32" ]; then
  checkJavaLinux
  checkCefPrebuiltLinux
  printGYPGenerator

  "$TOOL_DIR/gyp_chromium" --toplevel-dir="$PROJECT_DIR" --depth="$TOOL_DIR" \
      -Djava_include_path="$JAVA_INCLUDE_PATH" -Dcef_prebuilt_path="$APP_PATH" \
      -Dtarget_arch=ia32 \
      "$PROJECT_DIR/javacef.gyp"
fi

#if [[ "$GYP_GENERATORS" == *make* ]]; then
#  echo "Instructions to use make:"
#  echo "make"
#  echo "make BUILDTYPE=Release"
#fi
