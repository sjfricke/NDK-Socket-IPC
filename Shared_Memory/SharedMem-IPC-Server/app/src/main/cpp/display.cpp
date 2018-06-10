/*
 * This display file is design to do all the graphical UI display related stuff
 * This is to keep the actual IPC code separate for better understanding
 */
#include "display.h"

bool IsNDKReady(void) {
	// add prep logic

	// 4 bytes per pixel of max screen size
	color_buffer = (uint8_t*)malloc(MAX_SCREEN_SIZE * 4);
	if (color_buffer == NULL) {
		LOGE("Cant allocated color_buffer");
		exit(-1);
	}

	// Fills the buffer to green
	uint32_t green = 0x0000FF00;

	// 4K screen is 4096 x 2160
	int height = 4096;
	int width = 2160;

	for (int i = 0; i < height; i++) {;
		for (int j = 0; j < width; j++) {
			memcpy(color_buffer + (((i * width) + j) * 4), &green, sizeof(uint32_t));
		}
	}

	return true;
}


// Takes the native_window and sets a color
// This is so we can know the IPC worked
void setWindowWithBuffer() {

	ANativeWindow_acquire(native_window);
	ANativeWindow_Buffer buffer;
	if (ANativeWindow_lock(native_window, &buffer, nullptr) < 0) {
		LOGE("Failed to lock native window");
		return;
	}

	LOGI("/// H-W-S-F: %d, %d, %d, %d", buffer.height, buffer.width, buffer.stride, buffer.format);

	// assuming format was set to 4 bytes per pixel and not 565 mode
	memcpy(buffer.bits, color_buffer, (buffer.height * buffer.stride * 4));

	ANativeWindow_unlockAndPost(native_window);
	ANativeWindow_release(native_window);
}

void copyToColorBuffer(uint8_t* new_buffer, uint32_t size) {
	memcpy(color_buffer, new_buffer, size);
}

void cleanup() {
	free(color_buffer);
}

// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd) {
	switch (cmd) {
		case APP_CMD_INIT_WINDOW:
			native_window = app->window;

			// Here we set the buffer to use RGBX_8888 as default might be; RGB_565
			ANativeWindow_setBuffersGeometry(app->window,
											 ANativeWindow_getHeight(app->window),
											 ANativeWindow_getWidth(app->window),
											 WINDOW_FORMAT_RGBX_8888);

			setWindowWithBuffer();

			break;
		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			break;
		default:
			LOGI("event not handled: %d", cmd);
	}
}