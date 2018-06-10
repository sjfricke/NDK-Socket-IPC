# AHardwareBuffer IPC

This is a working demo using `AHardwareBuffer` API to send shared data between two processes.

This works by having the Client app `AHardwareBuffer_allocate()` and then writing to the shared memory upon picking a color (it generates the gradient pattern you see). From here the Server app upons an active window will see the data has been sent via `AHardwareBuffer_recvHandleFromUnixSocket()` and then write that memory to the `ANativeWindow_Buffer` which is then displayed on the screen.

![demo](../demo2.gif)
