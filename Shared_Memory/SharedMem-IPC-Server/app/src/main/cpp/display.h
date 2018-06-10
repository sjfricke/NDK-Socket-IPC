/*
 * This display file is design to do all the graphical UI display related stuff
 * This is to keep the actual IPC code separate for better understanding
 */
#ifndef SHAREDMEM_IPC_SERVER_DISPLAY_H
#define SHAREDMEM_IPC_SERVER_DISPLAY_H

#include <android_native_app_glue.h>
#include <android/native_window.h>
#include <android/looper.h>
#include <android/log.h>

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


// 4K screen is 4096 x 2160 so have buffer for up to 4K (assuming same NativeWindow stride)
#define MAX_SCREEN_SIZE 8847360

static ANativeWindow* native_window;

// The buffer that fills the screen
static uint8_t* color_buffer;

bool IsNDKReady(void);

void setWindowWithBuffer(void);

void copyToColorBuffer(uint8_t* new_buffer, uint32_t size);

void cleanup(void);

// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd);

#endif //SHAREDMEM_IPC_SERVER_DISPLAY_H
