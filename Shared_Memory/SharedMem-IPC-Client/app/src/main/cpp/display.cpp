/*
 * This display file is design to do all the graphical UI display related stuff
 * This is to keep the actual IPC code separate for better understanding
 */
#include "display.h"

bool IsNDKReady(void) {
	// add prep logic
	return true;
}

// Takes the native_window and sets a color
// This is so we can know what square to click
void setColorSections(ANativeWindow* native_window) {

	ANativeWindow_acquire(native_window);
	ANativeWindow_Buffer buffer;
	if (ANativeWindow_lock(native_window, &buffer, nullptr) < 0) {
		LOGE("Failed to lock native window");
		return;
	}

	LOGI("/// H-W-S-F: %d, %d, %d, %d", buffer.height, buffer.width, buffer.stride, buffer.format);

	// 3 columns of color
	for (int i = 0; i < buffer.height; i++) {
		for (int j = 0; j < buffer.stride; j++) {
			if (j < buffer.width / 3) {
				memcpy((char *) buffer.bits + (((i * buffer.stride) + j) * 4), &color_wheel[0], sizeof(uint32_t));
			} else if (j < buffer.width * 2 / 3) {
				memcpy((char *) buffer.bits + (((i * buffer.stride) + j) * 4), &color_wheel[1], sizeof(uint32_t));
			} else {
				memcpy((char *) buffer.bits + (((i * buffer.stride) + j) * 4), &color_wheel[2], sizeof(uint32_t));
			}
		}
	}

	// get height and stride while having window buffer
	screen_height = buffer.height;
	screen_stride = buffer.stride;

	ANativeWindow_unlockAndPost(native_window);
	ANativeWindow_release(native_window);
}

// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd) {
	switch (cmd) {
		case APP_CMD_INIT_WINDOW:
			// The window is being shown, get it ready.
			LOGI( "Width: %d", ANativeWindow_getWidth(app->window));
			LOGI( "Height: %d", ANativeWindow_getHeight(app->window));

			// Here we set the buffer to use RGBX_8888 as default might be; RGB_565
			ANativeWindow_setBuffersGeometry(app->window,
											 ANativeWindow_getHeight(app->window),
											 ANativeWindow_getWidth(app->window),
											 WINDOW_FORMAT_RGBX_8888);

			setColorSections(app->window);

			break;
		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			break;
		default:
			LOGI("event not handled: %d", cmd);
	}
}