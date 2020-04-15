#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "key.h"

#define PORT 8080

int main(int argc, char *argv[]) {
	int server_fd, new_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket failed.\n");
		return errno;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	memset(address.sin_zero, '\0', sizeof(address.sin_zero));
	if (bind(server_fd, (struct sockaddr *)&address, (socklen_t)addrlen) < 0) {
		perror("Bind failed.\n");
		return errno;
	}
	if (listen(server_fd, 1) < 0) {
		perror("Listen failed.\n");
		return errno;
	}
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
		perror("Accept failed.\n");
		return errno;
	}
	int fd_new = open("plain_text_got_back", O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd_new < 0) {
		perror("Open failed for output data file.\n");
		return errno;
	}
	uint16_t plain_text;
	uint16_t cipher_text;
	uint16_t bit_flips;
	if ((KEY & 0x0001) ^ (KEY & 0x8000)) {
		bit_flips = 0x5555;
	}
	else {
		bit_flips = 0xAAAA;
	}
	int num_read = 0;
	while (1) {
		if ((num_read = read(new_socket, &cipher_text, 2)) < 0) {
			perror("Read failed at server.\n");
			return errno;
		}
		else if (num_read == 0) {
			break;
		}
		/*
		if (close(new_socket) < 0) {
			perror("Close failed for obtained socket.\n");
			return errno;
		}
		*/
		int size = 2;
		if (num_read == 1) {
			size = 1;
		}
		plain_text = cipher_text ^ KEY ^ bit_flips;
		if (write(fd_new, &plain_text, size) < 0) {
			perror("Write failed into output data file.\n");
			return errno;
		}
	}
	if (close(fd_new) < 0) {
		perror("Close failed for output data file.\n");
		return errno;
	}
	if (close(server_fd) < 0) {
		perror("Close failed for initial socket.\n");
		return errno;
	}
	return 0;
}
