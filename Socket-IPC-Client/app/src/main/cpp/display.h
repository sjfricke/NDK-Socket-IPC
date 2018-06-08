/*
 * This display file is design to do all the graphical UI display related stuff
 * This is to keep the actual IPC code separate for better understanding
 */
#ifndef SOCKET_IPC_CLIENT_DISPLAY_H
#define SOCKET_IPC_CLIENT_DISPLAY_H

#include <android_native_app_glue.h>
#include <android/native_window.h>
#include <android/looper.h>
#include <android/log.h>

#include <string.h>
#include <pthread.h>
#include <stdint.h>

// Android log function wrappers
static const char* kTAG = "ClientIPC";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

const uint32_t color_wheel[4] = {
	0x000000FF, // red
	0x0000FF00, // green
	0x00FF0000, // blue
	0x00000FFFF // yellow
};

bool IsNDKReady(void);

void setColorSections(ANativeWindow* native_window);

// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd);

#endif //SOCKET_IPC_CLIENT_DISPLAY_H
