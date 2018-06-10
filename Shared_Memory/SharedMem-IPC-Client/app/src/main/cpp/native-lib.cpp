#include "display.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <android/sharedmem.h>
#include <sys/mman.h>

// Can be anything if using abstract namespace
#define SOCKET_NAME "sharedServerSocket"
#define BUFFER_SIZE 32

// Shared Memory has file descriptor we need to get to other app
// Also need size of data inside shared memory
static struct shared_mem {
	int fd = -1;
	int size = 0;
} shared_mem;
char *shared_buffer;

static int data_socket;
static struct sockaddr_un server_addr;

void setupClient(void) {
	char socket_name[108]; // 108 sun_path length max

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

void setSharedMem(uint8_t shift) {

	// Set up shared memory for first time
	if (shared_mem.fd < 0) {
		// Set up shared memory for 4 bytes per pixed
		shared_mem.size = getScreenHeight() * getScreenStride() * 4;

		shared_mem.fd = ASharedMemory_create("colorBuffer", shared_mem.size);

		shared_buffer = (char *)mmap(NULL, shared_mem.size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_mem.fd, 0);
	}

	//Produces a gradient pattern, uses shift to set as red, blue, or green
	for (int i = 0; i < getScreenHeight(); i++) {
		uint32_t color = imaxabs(255 - (i % 510)) << shift;
		for (int j = 0; j < getScreenStride(); j++) {
			memcpy(shared_buffer + (((i * getScreenStride()) + j) * 4), &color, sizeof(uint32_t));
		}
	}

}

void sendSharedMem() {
	int ret;
	uint8_t buffer[BUFFER_SIZE];
	uint8_t ret_buffer[BUFFER_SIZE];

	memcpy(buffer, &shared_mem, sizeof(shared_mem));

	ret = write(data_socket, &shared_mem, BUFFER_SIZE);
	if (ret < 0) {
		LOGE("write: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	ret = read(data_socket, ret_buffer, BUFFER_SIZE);
	if (ret < 0) {
		LOGE("read: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	LOGI("Return: %s", (char*)ret_buffer);
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