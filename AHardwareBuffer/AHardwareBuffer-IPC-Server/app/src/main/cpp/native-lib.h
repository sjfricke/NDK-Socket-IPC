/*
 * This display file is design to do all the graphical UI display related stuff
 * This is to keep the actual IPC code separate for better understanding
 */
#ifndef AHARDWAREBUFFER_IPC_SERVER_H
#define AHARDWAREBUFFER_IPC_SERVER_H

#include <android_native_app_glue.h>
#include <android/native_window.h>
#include <android/looper.h>
#include <android/log.h>
#include <android/hardware_buffer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <cmath>

// Android log function wrappers
static const char* kTAG = "ServerIPC";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

// Can be anything if using abstract namespace
#define SOCKET_NAME "sharedServerSocket"

bool IsNDKReady(void);

void setWindowWithBuffer(void);

void* setupServer(void* na);

// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd);

#endif //AHARDWAREBUFFER_IPC_SERVER_H
