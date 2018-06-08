# NDK-Socket-IPC

This this a experiment to see different forms of IPC (inter-process communication) for native apps not exposed to the Android Binder SDK class. This app has a server app that is one of 4 colors and a client app that changes the color.

![Demo gif](demo.gif)

> Disclaimer: The code is messy AF, was a quick prototype, for a friend as a proof of concept. Also have not profiled and not sure if there is better way of doing IPC in native Android app, if so please share since a Google search shows very little on topic

## The 3 Methods

1. [Unix Sockets](Unix_Sockets)
  - Good o'l C style Unix Sockets
  - Works for any version of Android
2. [Shared Memory](Shared_Memory)
  - Using the shared memory API [<android/sharedmem.h>](https://developer.android.com/ndk/reference/group/memory)
  - Android 8.0 (API 26) needed to run
3. [AHardwareBuffer](AHardwareBuffer)
  - Using the native hardware buffer API [<android/hardware_buffer.h>](https://developer.android.com/ndk/reference/group/native-activity)
  - Android 8.0 (API 26) needed to run

## How to run

- Each folder has a Client and Server folder each containing an Android Studio project.
- Open both and install on device.
- Open server app first (currently working on more robust example)
- Open client app and pick color
- Either switch back to server app to see change of color or open LogCat and view the print out
