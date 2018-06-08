# Unix Socket IPC

This example uses `<sys/socket>` sockets lib to handle communication between two app processes

## Things to Note

- Using `AF_INET` instead of `AF_UNIX` because just found out that there is a non internet based socket designed for Unix IPC purposes. Need to look into being able to write and read from same file descriptor and how it would get sent over
