#include <unistd.h>
#include <errno.h>

int main(){
	ssize_t size = write(1,"Hello, World!\n",14);
	if (size < 0){
		perror("error");
		return errno;
	}
	return 0;
}
