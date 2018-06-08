#include "display.h"

void setupClient(void) {
}

void sendColor(uint8_t color) {

}

// Handles input touches to the screen
int32_t handle_input(struct android_app* app, AInputEvent* event) {
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		if (AInputEvent_getSource(event) == AINPUT_SOURCE_TOUCHSCREEN) {
			if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_DOWN) {
				float x = AMotionEvent_getX(event,0);
				float y = AMotionEvent_getY(event,0);
				int32_t h_width = ANativeWindow_getWidth(app->window)/2;
				int32_t h_height = ANativeWindow_getHeight(app->window)/2;
				LOGI("X: %f  --   Y: %f", x, y);

				// Sends color depending on region
				if (x < h_width && y < h_height) { sendColor(0); }
				else if (x > h_width && y < h_height) { sendColor(1); }
				else if (x < h_width && y > h_height) { sendColor(2); }
				else { sendColor(3); }
			}
		}
	}
	return 1;
}

// Main Function
void android_main(struct android_app* app) {

	// Set the callback to process system events
	app->onAppCmd = handle_cmd;
	app->onInputEvent = handle_input;

	// Used to poll the events in the main loop
	int events;
	android_poll_source* source;

	setupClient();

	// Main loop
	do {
		if (ALooper_pollAll(IsNDKReady() ? 1 : 0, nullptr,
							&events, (void**)&source) >= 0) {
			if (source != NULL) source->process(app, source);
		}
	} while (app->destroyRequested == 0);

	LOGI( "GAME OVER");
}