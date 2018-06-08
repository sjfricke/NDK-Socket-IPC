#include <android_native_app_glue.h>
#include <android/native_window.h>
#include <android/looper.h>
#include <android/log.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>

#define DEFAULT_PORT 5000
#define MSG_SIZE 512

// Android log function wrappers
static const char* kTAG = "ClientIPC";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

bool IsNDKReady(void) {
	// add prep logic
	return true;
}
const uint32_t color_wheel[4] = {
	0x000000FF, // red
	0x0000FF00, // green
	0x00FF0000, // blue
	0x00000FFFF // yellow
};

int mySocket; // holds ID of the socket
struct sockaddr_in serv; // object of server to connect to
char* hostIP = "127.0.0.1";

void setupClient(void) {
	// Create socket
	// AF_INET refers to the Internet Domain
	// SOCK_STREAM sets a stream to send data
	// 0 will have the OS pick TCP for SOCK_STREAM
	mySocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mySocket < 0) { LOGE ("Could not create socket"); }

	serv.sin_addr.s_addr = inet_addr(hostIP); // sets IP of server
	serv.sin_family = AF_INET; // uses internet address domain
	serv.sin_port = htons(DEFAULT_PORT); // sets PORT on server

	// Connect to remote server with socket
	int status = connect(mySocket, (struct sockaddr *)&serv , sizeof(serv));
	if (status < 0) { LOGE("Connection error"); }

	LOGE("CONNECTED!!");
}

void sendColor(uint8_t color) {
	char message[MSG_SIZE];
	char server_reply[MSG_SIZE];

	memcpy(&message[0], &color, sizeof(uint8_t));

	int status =  send(mySocket, message , MSG_SIZE, 0);
	if (status < 0) { LOGE("Send failed\n"); }

	// receive a reply from the server
	status = recv(mySocket, server_reply, MSG_SIZE, 0);
	if (status < 0) { LOGE("recv failed\n"); }
	LOGI("%s", server_reply);
}

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

	// set top left red, top right green, bottom left blue, bottom right yellow
	for (int i = 0; i < buffer.height; i++) {

		if (i < buffer.height / 2) {
			for (int j = 0; j < buffer.stride; j++) {
				if (j < buffer.width / 2) {
					memcpy((char *) buffer.bits + (((i * buffer.stride) + j) * 4), &color_wheel[0], sizeof(uint32_t));
				} else {
					memcpy((char *) buffer.bits + (((i * buffer.stride) + j) * 4), &color_wheel[1], sizeof(uint32_t));
				}
			}
		} else {
			for (int j = 0; j < buffer.stride; j++) {
				if (j < buffer.width / 2) {
					memcpy((char *) buffer.bits + (((i * buffer.stride) + j) * 4), &color_wheel[2], sizeof(uint32_t));
				} else {
					memcpy((char *) buffer.bits + (((i * buffer.stride) + j) * 4), &color_wheel[3], sizeof(uint32_t));
				}
			}
		}
	}

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


