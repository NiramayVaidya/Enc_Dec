#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "key.h"

#define MAX_CIPHER_TEXT_SIZE 1024

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: ./decryption <filename>\n");
		exit(0);
	}
	int fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror("Open failed for input file. Check if it has been given read permissions.\n");
		return errno;
	}
	uint16_t cipher_text[MAX_CIPHER_TEXT_SIZE];
	int pos = 0;
	int num_read = 0;
	while ((num_read = read(fd, &cipher_text[pos++], 2)) != 0) {
		if (num_read == -1) {
			perror("Read failed for input file. Check if it has been given read permissions.\n");
			return errno;
		}
	}
	int fd_new = open("plain_text_got_back", O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd_new == -1) {
		perror("Open failed for decrypted data file.\n");
		return errno;
	}
	int end = lseek(fd, 0, SEEK_END);
	if (end == -1) {
		perror("Lseek failed for input file.\n");
		return errno;
	}
	pos = end / 2;
	uint16_t plain_text;
	uint16_t bit_flips;
	if ((KEY & 0x0001) ^ (KEY & 0x8000)) {
		bit_flips = 0x5555;
	}
	else {
		bit_flips = 0xAAAA;
	}
	for (int i = 0; i < pos; i++) {
		plain_text = cipher_text[i] ^ KEY ^ bit_flips;
		if (write(fd_new, &plain_text, 2) == -1) {
			perror("Write failed into decryption data file.\n");
			return errno;
		}
	}
	// char newline = '\n';
	// write(fd_new, &newline, 1);
	if (close(fd) == -1) {
		perror("Close failed for input file.\n");
		return errno;
	}
	if (close(fd_new) == -1) {
		perror("Close failed for decryption data file.\n");
		return errno;
	}
	return 0;
}
