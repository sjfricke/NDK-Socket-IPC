#include "native-lib.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <android/hardware_buffer.h>
static bool AHardwareBuffer_set = false;
static AHardwareBuffer* h_buffer;
static AHardwareBuffer_Desc h_buffer_desc;

// Can be anything if using abstract namespace
#define SOCKET_NAME "sharedServerSocket"
static int data_socket;

void setupClient(void) {
	char socket_name[108]; // 108 sun_path length max
	static struct sockaddr_un server_addr;

	data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (data_socket < 0) {
		LOGE("socket: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// NDK needs abstract namespace by leading with '\0'
	// Ya I was like WTF! too... http://www.toptip.ca/2013/01/unix-domain-socket-with-abstract-socket.html?m=1
	// Note you don't need to unlink() the socket then
	memcpy(&socket_name[0], "\0", 1);
	strcpy(&socket_name[1], SOCKET_NAME);

	// clear for safty
	memset(&server_addr, 0, sizeof(struct sockaddr_un));
	server_addr.sun_family = AF_UNIX; // Unix Domain instead of AF_INET IP domain
	strncpy(server_addr.sun_path, socket_name, sizeof(server_addr.sun_path) - 1); // 108 char max

	// Assuming only one init connection for demo
	int ret = connect(data_socket, (const struct sockaddr *) &server_addr, sizeof(struct sockaddr_un));
	if (ret < 0) {
		LOGE("connect: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	LOGI("Client Setup Complete");
}

void setupAHardwareBuffer(ANativeWindow* native_window) {

	// AFAIK the only way to get the window size is to acquire the window and get the
	// ANativeWindow_Buffer as below, happy to learn of other way
	ANativeWindow_acquire(native_window);
	ANativeWindow_Buffer buffer;
	if (ANativeWindow_lock(native_window, &buffer, nullptr) < 0) {
		LOGE("Failed to lock native window");
		return;
	}

	// Mimic window screen buffer info to our buffer_descripton
	h_buffer_desc = {
		.stride = static_cast<uint32_t>(buffer.stride),
		.height = static_cast<uint32_t>(buffer.height),
		.width = static_cast<uint32_t>(buffer.width),
		.layers = 1,
		.format = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM
	};

	int ret = AHardwareBuffer_allocate(&h_buffer_desc, &h_buffer);
	if (ret != 0) {
		LOGE("Failed to AHardwareBuffer_allocate");
	}

	ANativeWindow_unlockAndPost(native_window);
	ANativeWindow_release(native_window);

	AHardwareBuffer_set = true;
}

void setSharedMem(uint8_t shift) {
	int ret;
	void* shared_buffer;
	ret = AHardwareBuffer_lock(h_buffer,
						 AHARDWAREBUFFER_USAGE_CPU_WRITE_MASK,
						 -1, // no fence in demo
						 NULL,
						 &shared_buffer);
	if (ret != 0) {
		LOGE("Failed to AHardwareBuffer_lock");
	}

	//Produces a gradient pattern, uses shift to set as red, blue, or green
	for (int i = 0; i < h_buffer_desc.height; i++) {
		uint32_t color = imaxabs(255 - (i % 510)) << shift;
		for (int j = 0; j < h_buffer_desc.stride; j++) {
			memcpy((char*)shared_buffer + (((i * h_buffer_desc.stride) + j) * 4), &color, sizeof(uint32_t));
		}
	}

	ret = AHardwareBuffer_unlock(h_buffer, NULL);
	if (ret != 0) {
		LOGE("Failed to AHardwareBuffer_unlock");
	}
}

void sendSharedMem() {
	int ret = AHardwareBuffer_sendHandleToUnixSocket(h_buffer, data_socket);
	if (ret != 0) {
		LOGE("Failed to AHardwareBuffer_sendHandleToUnixSocket");
	}
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

	ANativeWindow_unlockAndPost(native_window);
	ANativeWindow_release(native_window);
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

				// try to compenstate for the touch screen's home and back button on phone
				if (y > ANativeWindow_getHeight(app->window) * 0.95) {
					return 1;
				}

				LOGI("X: %f  --   Y: %f", x, y);

				// Sends color depending on region
				if (x < h_width / 3) {
					setSharedMem(RED_SHIFT);
				} else if (x < h_width * 2 / 3) {
					setSharedMem(GREEN_SHIFT);
				} else {
					setSharedMem(BLUE_SHIFT);
				}

				sendSharedMem();
			}
		}
	}
	return 1;
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

			if (!AHardwareBuffer_set) {
				setupAHardwareBuffer(app->window);
			}

			setColorSections(app->window);

			break;
		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			break;
		default:
			LOGI("event not handled: %d", cmd);
	}
}

bool IsNDKReady(void) {
	// add prep logic
	return true;
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

	LOGI("GAME OVER");
}