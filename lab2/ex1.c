#include <unistd.h>
#include <errno.h>

int main(){
	ssize_t size = write(1,"Hello, World!\n",14);
	//primul argument e file descriptorul
	//in cazul nostru 1 (standard output)
	//al doilea este stringul de scris
	//al treilea este dimensiunea
	//returneaza numarul de bytes scris
	if (size < 0){
		perror("error");
		return errno;
	}
	return 0;
}
