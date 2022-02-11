#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFLEN 82
#define PORT 7615
#define ADDRESS "127.0.0.1"

char* prepare_packet(char* message) {

	memset(message, 0, BUFFLEN);
	message[0] = 129;
	message[1] = 2;
	message[2] = 11;
	message[3] = 5;
	for (int i = 0; i < BUFFLEN-1; i++) {

		message[81] ^= message[i];

	}
	sprintf(message+6, "zhizhin");

	return message;

}

int main() {
	int sockfd;
	char buffer[BUFFLEN];
	struct sockaddr_in servaddr;

	// Creating socket file descriptor
	// AF_INET      IPv4 Internet protocols
	// SOCK_DGRAM
	//              Supports datagrams (connectionless, unreliable messages of
	//              a fixed maximum length).
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( (sockfd) < 0 ) {
		printf("Socket descriptor creation failed\n");
	}

	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr(ADDRESS);

	char message[BUFFLEN];
	prepare_packet(message);

	printf("Sending packet.\n");
	sendto(sockfd, (const char *)message, BUFFLEN,
			MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
			sizeof(servaddr));
	printf("Sent packet.\n");

	unsigned int n, len;

	printf("Recieving packet.\n");
	n = recvfrom(sockfd, (char *)buffer, BUFFLEN, 
			MSG_WAITALL, (struct sockaddr *) &servaddr,
			&len);
	printf("Received.\n");

	buffer[n] = '\0';
	printf("Server : %s\n", buffer);

	close(sockfd);

	return 0;
}
