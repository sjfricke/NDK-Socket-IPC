#include "native-lib.h"

static AHardwareBuffer* h_buffer = nullptr;
static AHardwareBuffer_Desc h_buffer_desc;

static ANativeWindow* native_window;

// Server to get socket data with information of SharedMem's file descriptor
void* setupServer(void* na) {
	int ret;
	struct sockaddr_un server_addr;
	int socket_fd;
	int data_socket;
	char socket_name[108]; // 108 sun_path length max

	LOGI("Start server setup");

	// AF_UNIX for domain unix IPC and SOCK_STREAM since it works for the example
	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		LOGE("socket: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	LOGI("Socket made");

	// NDK needs abstract namespace by leading with '\0'
	// Ya I was like WTF! too... http://www.toptip.ca/2013/01/unix-domain-socket-with-abstract-socket.html?m=1
	// Note you don't need to unlink() the socket then
	memcpy(&socket_name[0], "\0", 1);
	strcpy(&socket_name[1], SOCKET_NAME);

	// clear for safty
	memset(&server_addr, 0, sizeof(struct sockaddr_un));
	server_addr.sun_family = AF_UNIX; // Unix Domain instead of AF_INET IP domain
	strncpy(server_addr.sun_path, socket_name, sizeof(server_addr.sun_path) - 1); // 108 char max

	ret = bind(socket_fd, (const struct sockaddr *) &server_addr, sizeof(struct sockaddr_un));
	if (ret < 0) {
		LOGE("bind: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	LOGI("Bind made");

	// Open 8 back buffers for this demo
	ret = listen(socket_fd, 8);
	if (ret < 0) {
		LOGE("listen: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	LOGI("Socket listening for packages");

	// Wait for incoming connection.
	data_socket = accept(socket_fd, NULL, NULL);
	if (data_socket < 0) {
		LOGE("accept: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	LOGI("Accepted data");
	// This is the main loop for handling connections
	// Assuming in example connection is established only once
	// Would be better to refactor this for robustness
	for (;;) {

		// Blocks until sent data
		ret = AHardwareBuffer_recvHandleFromUnixSocket(data_socket, &h_buffer);
		if (ret != 0) {
			LOGE("Failed to AHardwareBuffer_recvHandleFromUnixSocket");
		}

	}
	close(data_socket);
	close(socket_fd);

	return NULL;
}

// Takes the native_window and sets a color
// This is so we can know the IPC worked
void setWindowWithBuffer() {
	int ret;
	void* shared_buffer;

	ANativeWindow_acquire(native_window);
	ANativeWindow_Buffer buffer;
	if (ANativeWindow_lock(native_window, &buffer, nullptr) < 0) {
		LOGE("Failed to lock native window");
		return;
	}

	// need to pull out description of size info
	// currently assuming in demo, should use this to error check and stuff
	AHardwareBuffer_describe(h_buffer, &h_buffer_desc);

	LOGI("ANativeWindow_Buffer Size: %d", buffer.height * buffer.stride * 4);
	LOGI("AHardwareBuffer Size: %d", h_buffer_desc.height * h_buffer_desc.stride * 4);

	ret = AHardwareBuffer_lock(h_buffer,
							   AHARDWAREBUFFER_USAGE_CPU_READ_MASK,
							   -1, // no fence in demo
							   NULL,
							   &shared_buffer);
	if (ret != 0) {
		LOGE("Failed to AHardwareBuffer_lock");
	}

	// assuming format was set to 4 bytes per pixel and not 565 mode
	memcpy(buffer.bits, shared_buffer, (buffer.height * buffer.stride * 4));

	ret = AHardwareBuffer_unlock(h_buffer, NULL);
	if (ret != 0) {
		LOGE("Failed to AHardwareBuffer_unlock");
	}

	ANativeWindow_unlockAndPost(native_window);
	ANativeWindow_release(native_window);
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

			// Only can set window with screen is active
			if (h_buffer != nullptr) {
				setWindowWithBuffer();
			}

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

// Main function
void android_main(struct android_app* app) {

	// Set the callback to process system events
	app->onAppCmd = handle_cmd;

	// Used to poll the events in the main loop
	int events;
	android_poll_source* source;

	// Start server daemon on new thread
	pthread_t server_thread;
	pthread_create( &server_thread, NULL, setupServer, (void*) NULL);

	// Main loop
	do {
		if (ALooper_pollAll(IsNDKReady() ? 1 : 0, nullptr,
							&events, (void**)&source) >= 0) {
			if (source != NULL) source->process(app, source);
		}
	} while (app->destroyRequested == 0);

	LOGI("GAME OVER");
}


