#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

// Can be anything if using abstract namespace
#define SOCKET_NAME "serverSocket"
#define BUFFER_SIZE 16

void* setupServer(void* na) {
	int ret;
	struct sockaddr_un server_addr;
	int socket_fd;
	int data_socket;
	uint8_t buffer[BUFFER_SIZE];
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

		// Wait for next data packet
		ret = read(data_socket, buffer, BUFFER_SIZE);
		if (ret < 0) {
			LOGE("read: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		LOGI("Buffer: %d", buffer[0]);
		setColorSelected(buffer[0]);

		// Send back result
		sprintf((char*)buffer, "%d", "Got message");
		ret = write(data_socket, buffer, BUFFER_SIZE);
		if (ret < 0) {
			LOGE("write: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		// Close socket between accepts
	}
	close(data_socket);
	close(socket_fd);

	return NULL;
}

// Main function
void android_main(struct android_app* app) {

	// Set the callback to process system events
	app->onAppCmd = handle_cmd;

	// Used to poll the events in the main loop
	int events;
	android_poll_source* source;

	setColorSelected(0); // Start with red

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

	LOGI( "GAME OVER");
}


