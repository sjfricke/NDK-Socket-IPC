#include "display.h"

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

int mySocket; // holds ID of the socket
struct sockaddr_in serv; // object of server to connect to
char* hostIP = "127.0.0.1"; // Localhost since same device

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