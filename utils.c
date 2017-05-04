#include "utils.h"

int receive_message(message* msg, int fd) {
	int bytes_received;
	if ((bytes_received = recv(fd, msg, sizeof( message), 0)) <= 0) {
		if (bytes_received == 0) {
			fprintf(stderr, "Client disconnected.\n");
		}
		else {
			perror("Could not receive message");
		}
		return FALSE;
	}
	return TRUE;
}
void send_message(message msg, int socket) {
    if (send(socket, &msg, sizeof(struct message), 0) == -1) {
        perror("Failed to send a message to the socket");
    }
}
void shutdown_socket(int socket) {
	if (close(socket) < 0) {
		perror("Socket shutdown");
		exit(EXIT_FAILURE);
	}
}
