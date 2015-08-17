## Deprecated. Use javacef3 instead. ##

## Introduction ##

This wiki page has instructions about how to use or build in Linux.

## Use ##

  * Download all the sources using git command line. You can follow the steps on the [Source](http://code.google.com/p/javacef/source/checkout) tab. You can only download the "linux" folder.
  * Make sure you have an environment variable "JAVA\_HOME" to "path/to/jdk"
  * Make sure you have "java" and "javac" in your PATH.
  * Download and decompress cef binary release. For me, I used cef\_binary\_3.1364.1188\_linux64.
  * Link "Debug", "Release", "Resources" and "libcef\_dll" folders from cef release to the "linux" folder of this project.
  * To build in Debug, you can run the following in "linux" folder
```
make -j8
cd out/Debug
```
  * To build in Release, you can run the following in "linux" folder
```
make -j8 BUILDTYPE=Release
cd out/Release
```
  * Link libcef.so and generated libchromium\_loader.so to "path/to/jdk/jre/lib/amd64/ or i386/"
  * To run,
```
./run_javacef_sample
```

## Develop / Build from source ##