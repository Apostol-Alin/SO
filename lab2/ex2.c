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
		perror("Stat error");
		return errno;
	}
	if(file_open < 0){
		perror("Open error");
		return errno;
	}
	off_t size = sb.st_size;
	char* buff = malloc(size);
	ssize_t how_much_read = read(file_open, buff, size);
	if(how_much_read != size){
		perror("Error while reading from file");
		return errno;
	}
	close(file_open);
	file_open = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC);
	if(file_open < 0){
		perror("Open error");
		return errno;
	}
	ssize_t how_much_written = write(file_open, buff, size);
	if(how_much_written != size){
		perror("Error while writing in file");
		return errno;
	}
	close(file_open);
	free(buff);
	return 0;
}
