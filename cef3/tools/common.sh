
GYP_DIR="$TOOL_DIR/gyp"
PROJECT_DIR=`cd "$TOOL_DIR/.." && pwd`
JAVA_BIN="/bin/java"

checkJavaHomeMac() {
  if [ -z "$JAVA_HOME" ]; then
    echo "Error: JAVA_HOME is not set."
    echo
    echo "For example:"
    if [ "$1" -eq 64 ]; then
      echo "export JAVA_HOME=/Library/Java/JavaVirtualMachines/jdk1.7.0_45.jdk/Contents/Home"
    else
      echo "export JAVA_HOME=/System/Library/Java/JavaVirtualMachines/1.6.0.jdk/Contents/Home"
    fi
    echo
    echo "Suggested path:"
    if [ "$1" -eq 64 ]; then
      echo `/usr/libexec/java_home -Fa x86_64`
    else
      echo `/usr/libexec/java_home -Fa i386`
    fi
    exit 1
  fi
}

checkJavaIncludeMac() {
  if [ ! -d "$JAVA_INCLUDE_PATH" ];  then
    if [ ! -d "$JAVA_HOME/include" ];  then
      echo "Error: JAVA_INCLUDE_PATH is not set. Certain jni headers may not" \
           "be found at compile time."
      echo

      local INFO="option is to install Oracle JDK or OpenJDK, and set JAVA_INCLUDE_PATH to /path/to/jdk/include. The Java sources will still be compiled with the tools in JAVA_HOME."

      if [ "$1" -eq 64 ]; then
        echo "If you are using Oracle JDK or OpenJDK, the \"include\" folder is not found in"
        echo "$JAVA_HOME. Did you set JAVA_HOME correctly?"
        echo
        echo "If not, an $INFO"
      else
        echo "An $INFO"
      fi

      local TRY_PATH=`/usr/libexec/java_home -Fa x86_64 2>/dev/null`
      if [ ! -z "$TRY_PATH" ] && [ -d "$TRY_PATH/include" ]; then
        echo
        echo "Suggested path:"
        echo "$TRY_PATH/include"
      fi
      exit 1
    else
      JAVA_INCLUDE_PATH="$JAVA_HOME/include"
    fi
  fi
}

checkCefPrebuiltMac() {
  APP_PATH="$PROJECT_DIR/cef_runtime/$PLATFORM/cefclient.app"
  if [ ! -e "$APP_PATH" ]; then
    echo "Error: The prebuilt CEF release app is not found at $APP_PATH." \
         "Libraries may not be found at link time. Did you download the correct" \
         "CEF release and unzip it there?"
    exit 1
  fi
}

checkJavaRuntime32Mac() {
  "$JAVA_HOME$JAVA_BIN" -d32 -version 2> /dev/null
  if [ "$?" -ne 0 ]; then
    echo "The java in your JAVA_HOME does not support 32-bit."

    /usr/libexec/java_home -a i386 -F > /dev/null 2>&1
    if [ "$?" -ne 0 ]; then
      echo
      echo "Apple's JDK supports 32-bit. You can also install it:"
      echo "/usr/libexec/java_home -a i386 -R"
    else
      echo "You have 32-bit capable JDK installed. Suggested command:"
      echo "export JAVA_HOME=$(/usr/libexec/java_home -a i386)"
    fi
    exit 1
  fi
}

checkJarExist() {
  if [ ! -e "$BUILD_DIR/$JAR_NAME" ]; then
    echo "File $BUILD_DIR_RELATIVE/$JAR_NAME not found. Did you choose to run" \
         "the correct build type?"
    usage
    exit 1
  fi
}

printGYPGenerator() {
  echo "Generating $GYP_GENERATORS build files..."
}

checkJavaLinux() {
  if [ -z "$JAVA_HOME" ]; then
    echo "Error: JAVA_HOME environment variable is not set."
    exit 1
  fi

  if [ ! -d "$JAVA_HOME/include" ];  then
    echo "Error: The \"include\" folder is not found in $JAVA_HOME."
    exit 1
  fi

  JAVA_INCLUDE_PATH="$JAVA_HOME/include"
}

checkCefPrebuiltLinux() {
  APP_PATH="$PROJECT_DIR/cef_runtime/$PLATFORM"
  if [ ! -e "$APP_PATH/cefclient" ]; then
    echo "Warning: The prebuilt CEF release binaries are not found at $APP_PATH." \
         "Libraries may not be found at link time. Did you download the correct" \
         "CEF release and unzip it there?"
  fi
}

