# Shared Memory IPC

So after playing around I found that it **Should** be possible to use the Shared Memory API, but this is not the route you want. the `<android/sharedmem.h>` API was designed to share memory between processes like `<android/NeuralNetworks.h>` where you will call [ANeuralNetworksMemory_createFromFd()](https://developer.android.com/ndk/reference/group/neural-networks#group___neural_networks_1ga3510b07da0ab9626fb84688fb91112be) and the under lying Android NDK will take the file descriptor from there.

To finish this example, which I honestly will probably not do, is to share the file descriptor from the mapped memory of your `ASharedMemory_create()` call (which is really just a `shm_open()` call) and send use the Unix Domain Sockets to send the file descriptor to the other process using `sendmsg(2)` and `recvmsg(2)`

Here are some links if you still want to go this route

- https://keithp.com/blogs/fd-passing/
- http://poincare.matf.bg.ac.rs/~ivana/courses/ps/sistemi_knjige/pomocno/apue/APUE/0201433079/ch17lev1sec4.html
- https://gist.github.com/nazgee/2396992
- https://stackoverflow.com/questions/2358684/can-i-share-a-file-descriptor-to-another-process-on-linux-or-are-they-local-to-t?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa

## The better option

For the example of sending data across two seperate process, I recommend using the `<android/hardware_buffer.h>` API which came out with Android 8.0 as well with I assume these two purpose in mind... who knows with the NDK :D
