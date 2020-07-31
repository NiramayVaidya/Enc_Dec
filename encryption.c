#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* sudo chmod go-r key.h
 * sudo useradd temp (check /etc/passwd)
 * sudo chown temp key.h
 * vi key.h -> [permission denied] for read
 * TODO change the encryption method such that it works for different key sizes
 */

#define MAX_PLAIN_TEXT_SIZE 1024

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: ./encryption <filename>\n");
		exit(0);
	}
	int fd = open(argv[1], O_RDONLY);
	if (fd == -1) {
		perror("Open failed for input file. Check if it has been given read permissions.\n");
		return errno;
	}
	uint16_t plain_text[MAX_PLAIN_TEXT_SIZE];
	int pos = 0;
	int num_read = 0;
	while ((num_read = read(fd, &plain_text[pos++], 2)) != 0) {
		if (num_read == -1) {
			perror("Read failed for input file. Check if it has been given read permissions.\n");
			return errno;
		}
	}
	/* when the cipher_text file is created by giving permissions as 666 without
	 * the 0, it gets permissions as -w--wx--T (sticky bit has been set)
	 * TODO find out why the permissions are set like above even if 666 is
	 * provided
	 */
	int fd_new = open("cipher_text", O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd_new == -1) {
		perror("Open failed for encrypted data file.\n");
		return errno;
	}
	int end = lseek(fd, 0, SEEK_END);
	if (end == -1) {
		perror("Lseek failed for input file.\n");
		return errno;
	}
	pos = end / 2;
	uint16_t cipher_text;
	uint16_t bit_flips;
	if ((KEY & 0x0001) ^ (KEY & 0x8000)) {
		/* odd position bit flips
		 */
		bit_flips = 0x5555;
	}
	else {
		/* even position bit flips
		 */
		bit_flips = 0xAAAA;
	}
	for (int i = 0; i < pos; i++) {
		cipher_text = plain_text[i] ^ KEY ^ bit_flips;
		if (write(fd_new, &cipher_text, 2) == -1) {
			perror("Write failed into encryption data file.\n");
			return errno;
		}
	}
	char newline = '\n';
	write(fd_new, &newline, 1);
	if (close(fd) == -1) {
		perror("Close failed for input file.\n");
		return errno;
	}
	if (close(fd_new) == -1) {
		perror("Close failed for encryption data file.\n");
		return errno;
	}
	return 0;
}
