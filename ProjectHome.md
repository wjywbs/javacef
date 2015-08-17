## Introduction ##
Java CEF is now an open source project by Jingyi Wei. This project can embed Chromium browser in Java SWT with multitab browser support, cookies manipulation, tab settings, printing, back, forward, refresh buttons and enhanced file download. This project is based on [Chromium Embedded Framework](http://code.google.com/p/chromiumembedded/) (CEF).

## Background: Why javacef become open source? ##
I found a job in summer 2012 to make an interface to use Chromium browser with SWT in Java. It was my summer vacation after graduating from my high school in Qingdao, China. **I was 17 years old at that time. I've never signed any contract with the company**, and several months later, some of the core developers and coordinators left the company. So they don't need this any more. I kept this javacef in my computer for several months. However, I knew that it would be hard to find another company to sell this software. I knew it would be helpful for me to find an internship. I also knew that many people may struggle about creating Java bindings of CEF. I believed that things derived from open source project would be better to be open source. If I publish it, I may be able to receive some donation to pay for my college tuition in the US. Therefore, I published javacef in May, 2013.

[![](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=9XACJ27BHJ35U)

## javacef 3 ##
  * **Stable** on **Windows, Mac and Linux** with **32-bit and 64-bit support**.
  * CEF 3.1750.1738 on Windows, Mac and Linux
  * Act like a control in SWT.
  * Sandbox is enabled on Mac and Linux by default, but not on Windows yet.
  * Support alerts, dialogs, drop-down list, WebGL, etc.
  * Support developer tools and right click menus.
  * Support cookies manipulation and tab settings.
  * Easier to use. Share the same source code on all platforms.
  * Downloadable release builds will be available.
  * Easier to build and develop: You can generate MSVS, make, xcode and ninja projects.
  * For linux, only Ubuntu 12.04 is tested.
  * [How to use or build](CEF3Instructions.md).

## What's the difference between javacef and javachromiumembedded? ##
  * Javacef aims to embed CEF in SWT, while javachromiumembedded is mainly for AWT. Note that SWT and AWT can not be used at the same time on Mac, so if you are using SWT on Mac, this project works.
  * Javacef aims to act like a control and easy to embed as a browser. Low level C++ functions will not be exposed.
  * Javacef 3 is stable on all platforms, which means it runs cleanly and no error exit code is returned. It's less stable on exit if the web page has NPAPI plugins or extensive WebGL.
  * Javacef 3 support right-click menu, devtools and drop-down list (platform native controls).
  * Javacef does not use off-screen rendering, and does not have other dependencies.

## Previous: Windows CEF1 ##
  * **Stable**.
  * cef 1.1025.607 windows, Chrome 18, 32-bit only (cef\_binary\_1.1025.607\_windows)
  * Print, Audio, Video, Multitab support
  * Cookies manipulation, Tab settings and Enhanced file download
  * Take a little time to build from source.
  * Have some simple assembly code in C++ source.
  * You can download the release version and only need to change a little part to run. (Currently not supported. See instructions below.)
  * I didn't add copyright notices in Java sources, but all Java sources follows the license in the LICENSE file.
  * [How to use or build](WindowsInstructions.md).

> TODOs:
  * I think Google code changed all tabs in Java files to 8 spaces.
  * Some C++ files don't follow "Google C++ Code Style" to include headers.
  * Some platform dependent code should be placed in windows platform specific files.

## Previous: Linux (Deprecated. Use CEF3 above instead.) ##
  * Unstable.
  * Partial functions.
  * Easy to build from source.
  * Served as starter.
  * Currently cef\_binary\_3.1364.1188\_linux64.
  * [How to use or build](LinuxInstructions.md).