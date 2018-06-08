# NDK-Socket-IPC

This this a experiment to see if you can run two apps at once using Unix Sockets to communicate as an IPC (inter-process communication).

This was not designed an IPC replacement, but because I can't find any example online showing otherwise

## Conclusion

This method does work actually! If you LogCat the server application when running you will see the input from the client trigger it

## Three BIG things to consider

1. My color example fails because the server can't update the color since it can't get the ANativeWindow when in background. One could store the data for the example and check when the app comes back into foreground. Something I would like to look more into.
2. I found there is a `AF_UNIX` socket family instead of `AF_INET` so not sure if that would be a way better approuch
3. The code is messy AF, was a quick prototype, for a friend as a proof of concept
