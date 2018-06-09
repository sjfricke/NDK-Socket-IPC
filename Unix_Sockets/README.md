# Unix Socket IPC

This example uses `<sys/socket>` sockets lib to handle communication between two app processes. I use the `AF_UNIX` Unix Domain which sends the data via the kernel over a file descriptor compared to using the `AF_INET` IP Protocol.

## Things to Note

- The app example atm assumes you open the sever app, then the client app. I didn't build example to handle reconnects or polling for a valid server app in this example.
- You are can only from my testing write a Unix Domain file descriptor to internal storage which is useless since the other app can't see it. You can't write a Unix Domain socket file descriptor to the Public External Storage due to security risk and therefore use an abstracted namespace for the file descriptor. Here is a [GREAT PAPER](http://web.eecs.umich.edu/~yurushao/pubs/sinspector_ccs2016.pdf) on the security of Unix Domain IPC in Android!
  - [Slides for paper](http://web.eecs.umich.edu/~yurushao/pubs/sinspector_ccs2016_slides.pdf)
