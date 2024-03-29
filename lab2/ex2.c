#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
	int file_open = open(argv[1], O_CREAT | O_RDONLY);
	// functie open returneaza file_descriptor-ul asociat fisierului
	// flag-ul O_CREAT spune ca daca fisierul nu exista, sa se creeze
	// flag-ul O_RDONLY restrictioneaza doar spre citire din fisier
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
	//ne luam cati bytes avem nevoie
	char* buff = malloc(size);
	//alocam dinamic un buffer
	ssize_t how_much_read = read(file_open, buff, size);
	//citim din fisier
	//functia intoarce cati bytes s-au citit
	if(how_much_read != size){
		perror("Error while reading from file");
		return errno;
	}
	close(file_open);
	//inchidem fisierul cu fd-ul file_open
	file_open = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC);
	//deschidem fisierul destinatie
	// O_WRONLY restrictioneaza doar spre scriere
	// O_TRUNC : daca a fost deschis fisierul cu drept de scriere
	// si fisierul exista deja il trunchiaza la lungime 0 BYTES
	if(file_open < 0){
		perror("Open error");
		return errno;
	}
	ssize_t how_much_written = write(file_open, buff, size);
	//scriem in fisier si vedem cat am scris
	if(how_much_written != size){
		perror("Error while writing in file");
		//daca am scris mai putin decat am avut in fiserul sursa
		//s-a produs o eroare
		return errno;
	}
	close(file_open);
	//inchidem fisierul
	free(buff);
	//dezalocam memoria
	return 0;
}
