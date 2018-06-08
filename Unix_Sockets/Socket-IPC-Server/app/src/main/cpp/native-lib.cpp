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

struct sockaddr_in dest; // socket info about the machine connecting to us
struct sockaddr_in server; // socket info about our server
int mySocket;            // socket used to listen for incoming connections
int consocket;           // used to hold status of connect to socket
socklen_t socksize = sizeof(struct sockaddr_in);

void* setupServer(void* na) {
	char receiveMsg[MSG_SIZE]; // message buffer
	char returnMsg[] = "message received";

	memset(&server, 0, sizeof(server)); // zero the struct before filling the fields
	server.sin_family = AF_INET; // set to use Internet address family
	server.sin_addr.s_addr = htonl(INADDR_ANY); // sets our local IP address
	server.sin_port = htons(DEFAULT_PORT); // sets the server port number

	mySocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mySocket < 0) { LOGE("ERROR: Opening socket"); }

	// bind server information to mySocket
	int status = bind(mySocket, (struct sockaddr *)&server, sizeof(struct sockaddr));

	// checks for TIME_WAIT socket
	// when daemon is closed there is a delay to make sure all TCP data is propagated
	if (status < 0) {
		LOGE("ERROR opening socket: %d , possible TIME_WAIT\n", status);
		exit(-1);
	}

	// start listening, allowing a queue of up to 1 pending connection
	listen(mySocket, 1);

	LOGI("SERVER UP AND READY");

	for (;;) { // keeps daemon running forever

		// blocks until a TCP handshake is made
		consocket = accept(mySocket, (struct sockaddr *)&dest, &socksize);

		// main loop to wait for a request
		while(consocket) {

			// used to get the message of length MSG_SIZE
			int msgSize = recv(consocket, receiveMsg, MSG_SIZE , 0);
			if (msgSize < 0) { LOGE("ERROR on recv"); }
			else if (msgSize == 0) { break; } // clients dropped connection from socket

			setColorSelected((uint8_t)receiveMsg[0]);
			LOGI("Color Index: %d", (uint8_t)receiveMsg[0]);

			// clears receive message buffer
			memset(receiveMsg, 0, MSG_SIZE);

			// sends back response message
			status = send(consocket, returnMsg, strlen(returnMsg), 0);
			if (status < 0) { LOGE("ERROR on send"); }

		} // end of connection while loop

		close(consocket); // ends current TCP connection and frees server thread
		LOGE("client dropped connection");

	} // end forever loop

	// clean up on close if reached
	free(receiveMsg);
	close(mySocket);

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


