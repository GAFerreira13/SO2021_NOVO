#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define STR_LEN 100

int main (int argc, char *argv[]) {

	if (argc != 2) {
		printf("Wrong number of arguments! Usage: ./open device_name \n");
	}

	char aux [STR_LEN]= "/dev/";
	int f = open (strcat(aux,argv[1]), O_RDWR);

	if (f < 0) {
		printf ("Could not open device \"%s\"! \n", argv[1]);
		return -1;
	}

	printf ("Enter the input string! Max: %d bytes. \n",STR_LEN );

	fgets (aux, STR_LEN, stdin);
	int char_num = read ( f, aux, strlen(aux));

	if (char_num < 0) {
		printf ("Errro writing in device! \n");
	} else if (char_num != strlen(aux)) {
		printf ("Wrong number of bytes written! %d %d \n", char_num, strlen(aux));
	} else {
		printf ("Success! \n");
	}


	if (close(f)) {
		printf ("Error closing file %d! \n", f);
		return -1;
	}

	printf ("Device \"%s\" closed successfully! \n", argv[1]);
	return 0;

}
