#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
	int file_open = open(argv[1], O_CREAT | O_RDONLY);
	struct stat sb;
	if(stat(argv[1], &sb)){
		perror("error");
		return errno;
	}
	if(file_open < 0)
		return errno;
	off_t size = sb.st_size;
	char* buff = malloc(size);
	ssize_t how_much_read = read(file_open, buff, size);
	if(how_much_read < 0)
		return errno;
	close(file_open);
	file_open = open(argv[2], O_CREAT | O_WRONLY);
	if(file_open < 0)
		return errno;
	ssize_t how_much_written = write(file_open, buff, size);
	if(how_much_written != size)
		return errno;
	free(buff);
	return 0;
}
