#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include "key.h"

#define PORT 8080

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: ./encryption <filename>\n");
		exit(0);
	}
	int client_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket failed.\n");
		return errno;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	memset(address.sin_zero, '\0', sizeof(address.sin_zero));
	if (connect(client_fd, (struct sockaddr *)&address, (socklen_t)addrlen) < 0){
		perror("Connect failed.\n");
		return errno;
	}
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("Open failed for input file. Check if it has been given read permissions.\n");
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
	while ((num_read = read(fd, &plain_text, 2)) != 0) {
		if (num_read < 0) {
			perror("Read failed for input file. Check if it has been given read permissions.\n");
			return errno;
		}
		cipher_text = plain_text ^ KEY ^ bit_flips;
		int size = 2;
		if (num_read == 1) {
			size = 1;
		}
		if (write(client_fd, &cipher_text, size) < 0) {
			perror("Write failed at client.\n");
			return errno;
		}
	}
	if (close(fd) < 0) {
		perror("Close failed for input file.\n");
		return errno;
	}
	if (close(client_fd) < 0) {
		perror("Close failed for initial socket.\n");
		return errno;
	}
	return 0;
}
