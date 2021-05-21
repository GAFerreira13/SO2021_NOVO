#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define STR_LEN 100

int main(int argc, char *argv[]) {

	if (argc != 2) {
		printf("Wrong number of arguments! Usage: ./open device_name \n");
		return -1;
	}

	char aux[STR_LEN] = "/dev/";
	int f = open(strcat(aux, argv[1]), O_RDWR);

	if (f < 0) {
		printf("Could not open device \"%s\"! \n", argv[1]);
		return -1;
	}

	printf("Enter the input string! Max: %d bytes. \n", STR_LEN);


	if (read(f, aux, STR_LEN) != -1) {
		printf("Received string: %s \n", aux);
	} else	{
		printf("Error reading char\n");
		return -1;
	}


	if (close(f))	{
		printf("Error closing file %d! \n", f);
		return -1;
	}

	printf("Device \"%s\" closed successfully! \n", argv[1]);
	return 0;
}
