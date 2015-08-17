## General ##

The source code is located in `cef3/`. You should have `svn`, `git` and `python` installed. Check out the source code with the steps on the [Source](http://code.google.com/p/javacef/source/checkout) tab.

You need to [download](http://www.magpcss.net/cef_downloads/) CEF client binary distribution corresponding to the versions in the `cef_runtime_version.txt` file. You may need to click the "older (deprecated) versions" link in the download page to find the correct version.

In cef version 3.1750.1738, sandbox is turned on by default on Mac and Linux. Please read the notes below for sandbox issues. Sandbox on Windows is not supported yet.

## Windows ##

  * Download the prebuilt CEF runtime. For example, if you are building 32-bit, you need to download `cef_binary_(version)_windows32_client.7z`, unzip it and put the cefclient.exe and all other files in `cef_runtime\win32`. Same to 64-bit.
  * Set `JAVA_HOME` environment variable. For me, it's "D:\Program Files (x86)\Java\jdk1.7.0\_45".
  * Open `tools\configure.bat`. GYP will be downloaded for the first run. Then select the target to generate the build files.

  * If you selected `MSVS`:
  * You can open `javacef.sln` in `cef3\` and build `javacef` target right away. This will build everything including the java files. You can also build "Release" and/or "x64".
  * Open `tools\run.bat` to run the java classes.
  * Known issue: If you build x64, you may need to rename `javacef3_win32.jar` to `javacef3_win64.jar` in `out\build_type`.

  * If you selected `ninja`:
```
cd path\to\javacef\cef3

# To build win32
ninja -C out\Debug
ninja -C out\Release

# You need to choose win64 in configure.bat to build 64-bit
# When using run.bat, choose ninja specific settings.
ninja -C out\Debug_x64
ninja -C out\Release_x64

# Open tools\run.bat to run the java classes.
```
  * Note that ninja does not work well with non-English language MSVC compiler. Include info will be outputted, which is not the intended behavior.

  * If you want to develop the java sources, you can open the `java` folder with eclipse and set up JVM arguments (read `tools\run.bat` for more info).

## Mac and Linux ##

  * Download the prebuilt CEF runtime. For example, if you are building 64-bit Mac, you need to download `cef_binary_(version)_macosx64_client.7z` and put the `cefclient.app` in `cef_runtime/mac64`. If you are building 64-bit Linux, you need to download `cef_binary_(version)_linux64_client.7z` and put cefclient and all other files in `cef_runtime/linux64`.
```
cd path/to/javacef/cef3

# Choices are mac64|mac32|linux64|linux32.
#
# This script will generate ninja build files if ninja is detected
# in $PATH, otherwise make files are generated. "Generating make/ninja
# build files..." will be outputted.
tools/configure.sh mac64

# Optional: Force generate make files.
# GYP_GENERATORS=make tools/configure.sh mac64

# ninja build files
ninja -C out/Debug
ninja -C out/Release

# make files - pass -j(num) to build in parallel
make
make BUILDTYPE=Release

# Run SampleBrowserSWT
# You can change run.sh to open SampleWindow.
tools/run.sh mac64
tools/run.sh mac64 release
```
  * To turn off sandbox after cef 3.1750.1738, add `settings.no_sandbox = true;` after `settings.log_severity = LOGSEVERITY_DISABLE;` in chromium\_loader\_mac/linux.cpp

## Mac Specific ##
  * Xcode - You need to open `chromium_loader` project settings and change the library search path to `/path/to/Chromium Embedded Framework.framework/Libraries` (remove `-L` at start and add `/Libraries` at end). If you use Xcode > 5, change the compiler from unsupported to LLVM in all sub-projects. In order to debug, set java as executable and set up arguments and environment variables (read `tools\run.sh` for info).
```
# Generate Xcode projects
GYP_GENERATORS=xcode tools/configure.sh mac64
```
  * Notes on mac32 - Apple's JDK supports 32-bit. Oracle's JDK doesn't. The suggestion is to install both Apple's JDK and Oracle's JDK. Set `JAVA_HOME` to Apple's JDK, and set `JAVA_INCLUDE_PATH` to `/path/to/oracle/jdk/include`.
```
# This will output detailed instructions.
tools/configure.sh mac32
```
  * Notes on JVM - JVM signal handlers on Mac and Linux are overridden by CEF during `CefInitialize()`. The fix is to backup the signal handlers before `CefInitialize` and restore them after `CefInitialize`. The detailed reason was posted [here](https://code.google.com/p/javachromiumembedded/issues/detail?id=41#c32). Before the fix on 6/8/2014, accessing to null objects will not be handled by JVM and cause crashes. For example,
```
  if (folder.isDisposed() || folder.getItemCount() == 0)
    return;

  # Without the check above, this line will cause SIGSEGV.
  Chromium c = (Chromium) folder.getSelection().getControl();
```
  * Known issue with CEF 3.1547.1412: the browser sub process may not exit when a browser tab is closed.

## Linux Specific ##
  * Only Ubuntu 12.04 is tested. The [link](https://code.google.com/p/javacef/issues/detail?id=5) may be helpful to build on Arch Linux.
  * In cef 3.1750.1738, you may need to change the permission of the "chrome-sandbox" file to run the examples.
```
sudo chown root:root cef_runtime/linux64/chrome-sandbox
sudo chmod 4755 cef_runtime/linux64/chrome-sandbox
```
  * (This only applies to the code before 6/8/2014. The SWT libraries on all platforms are copied from the official release now.) The Linux SWT libraries are modified (`cef3/java/lib/linux/swt*.jar`). Because the JVM signal handlers are overridden by CEF, hovering the mouse cursor to the close button of `CTabFolder` will cause SIGSEGV. The changed file is:
```
diff --git a/org/eclipse/swt/custom/CTabFolder.java b/org/eclipse/swt/custom/CTabFolder.java
index 4bd2847..9b53c54 100644
--- a/org/eclipse/swt/custom/CTabFolder.java
+++ b/org/eclipse/swt/custom/CTabFolder.java
@@ -3581,7 +3581,7 @@ public void showSelection () {
 void _setToolTipText (int x, int y) {
        String oldTip = getToolTipText();
        String newTip = _getToolTip(x, y);
-       if (newTip == null || !newTip.equals(oldTip)) {
+       if (newTip == null || (oldTip != null && !newTip.equals(oldTip))) {
                setToolTipText(newTip);
        }
 }
```
```
# The swt.jar is changed in this way
cd org/eclipse/swt/custom
javac -target 1.2 -source 1.2 CTabFolder.java -g:none -cp /path/to/swt-4.3-gtk-linux-x86_64/src/

unzip swt.jar
# Replace CTabFolder.class in unzipped swt with the newly compiled CTabFolder.class.
cp ...
rm -r META-INF
jar cvf swt-fixed.jar -C swt .
```