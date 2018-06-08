/*
 * This display file is design to do all the graphical UI display related stuff
 * This is to keep the actual IPC code separate for better understanding
 */
#include "display.h"

static uint8_t color_selected;

void setColorSelected(uint8_t color) {
	color_selected = color;
}

bool IsNDKReady(void) {
	// add prep logic
	return true;
}

// Takes the native_window and sets a color
// This is so we can know the IPC worked
void setWindowColor(uint8_t color_index) {

	ANativeWindow_acquire(native_window);
	ANativeWindow_Buffer buffer;
	if (ANativeWindow_lock(native_window, &buffer, nullptr) < 0) {
		LOGE("Failed to lock native window");
		return;
	}

	LOGI("/// H-W-S-F: %d, %d, %d, %d", buffer.height, buffer.width, buffer.stride, buffer.format);

	// set each pixel
	for (int i = 0; i < buffer.height; i++) {
		for (int j = 0; j < buffer.stride; j++) {
			memcpy((char*)buffer.bits + (((i * buffer.stride) + j) * 4), &color_wheel[color_index], sizeof(uint32_t));
		}
	}

	ANativeWindow_unlockAndPost(native_window);
	ANativeWindow_release(native_window);
}


// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd) {
	switch (cmd) {
		case APP_CMD_INIT_WINDOW:
			native_window = app->window;
			// The window is being shown, get it ready.
			LOGI( "Width: %d", ANativeWindow_getWidth(app->window));
			LOGI( "Height: %d", ANativeWindow_getHeight(app->window));

			// Here we set the buffer to use RGBX_8888 as default might be; RGB_565
			ANativeWindow_setBuffersGeometry(app->window,
											 ANativeWindow_getHeight(app->window),
											 ANativeWindow_getWidth(app->window),
											 WINDOW_FORMAT_RGBX_8888);

			LOGI("Color index to set: %d", color_selected);
			setWindowColor(color_selected); // Set screen to color from color_wheel[]

			break;
		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			break;
		default:
			LOGI("event not handled: %d", cmd);
	}
}