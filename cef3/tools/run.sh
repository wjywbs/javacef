#!/bin/bash

usage() {
  echo "Usage: ./run.sh mac64|mac32|linux64|linux32 [debug|release]"
}

if [ -z "$1" ]; then
  usage
  exit 1
fi

PLATFORM=$1
BUILD_TYPE=$2

JAR_NAME="javacef3_$PLATFORM.jar"

TOOL_DIR="$( cd "$( dirname "$0" )" && pwd )"
. "$TOOL_DIR/common.sh"

if [ "$2" == "debug" ]; then
  BUILD_TYPE="Debug"
fi

if [ "$2" == "release" ]; then
  BUILD_TYPE="Release"
fi

if [ -z "$BUILD_TYPE" ]; then
  BUILD_TYPE="Debug"
fi

BUILD_DIR_RELATIVE="out/$BUILD_TYPE"
BUILD_DIR="$PROJECT_DIR/$BUILD_DIR_RELATIVE"
echo "Running $BUILD_TYPE build."

if [ "$PLATFORM" == "mac64" ]; then
  checkJavaHomeMac
  checkCefPrebuiltMac
  checkJarExist

  export DYLD_LIBRARY_PATH="$APP_PATH/Contents/Frameworks/Chromium Embedded Framework.framework"
  export JAVACEF_PATH="$PROJECT_DIR"

  "$JAVA_HOME$JAVA_BIN" -XstartOnFirstThread \
      -cp "$PROJECT_DIR/java/lib/mac/swt64.jar:$BUILD_DIR/$JAR_NAME" \
      -Djava.library.path="$BUILD_DIR:$DYLD_LIBRARY_PATH" \
      -Duser.dir="$PROJECT_DIR/java" \
      org.embedded.browser.SampleBrowserSWT
fi

if [ "$PLATFORM" == "mac32" ]; then
  echo "mac32 is no longer supported in cef 2526. Try 3.1750 branch instead."
  exit 1
fi

if [ "$PLATFORM" == "linux64" ]; then
  checkJavaLinux
  checkCefPrebuiltLinux
  checkJarExist

  export LD_LIBRARY_PATH="$APP_PATH/lib"
  export JAVACEF_PATH="$PROJECT_DIR"
  export CHROME_DEVEL_SANDBOX="$APP_PATH/chrome-sandbox"

  "$JAVA_HOME$JAVA_BIN" \
      -cp "$PROJECT_DIR/java/lib/linux/swt64.jar:$BUILD_DIR/$JAR_NAME" \
      -Djava.library.path="$BUILD_DIR/lib:$BUILD_DIR/lib.target" \
      -Duser.dir="$PROJECT_DIR/java" \
      org.embedded.browser.SampleBrowserSWT
fi

if [ "$PLATFORM" == "linux32" ]; then
  checkJavaLinux
  checkCefPrebuiltLinux
  checkJarExist

  export LD_LIBRARY_PATH="$APP_PATH/lib"
  export JAVACEF_PATH="$PROJECT_DIR"
  export CHROME_DEVEL_SANDBOX="$APP_PATH/chrome-sandbox"

  "$JAVA_HOME$JAVA_BIN" \
      -cp "$PROJECT_DIR/java/lib/linux/swt32.jar:$BUILD_DIR/$JAR_NAME" \
      -Djava.library.path="$BUILD_DIR/lib:$BUILD_DIR/lib.target" \
      -Duser.dir="$PROJECT_DIR/java" \
      org.embedded.browser.SampleBrowserSWT
fi

