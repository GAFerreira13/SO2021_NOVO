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



int main (int argc, char *argv[]) {

	if (argc != 2) {
		printf("Wrong number of arguments! Usage: ./open device_name \n");
	}

	char aux [16]= "/dev/";
	int f = open (strcat(aux,argv[1]), O_RDWR);
	if (f < 0) {
		printf ("Could not open device \"%s\"! \n", argv[1]);
		return -1;
	} else if (close(f)) {
		printf ("Error closing file %d! \n", f);
		return -1;
	} else {
		printf ("Device \"%s\" opened and closed successfully! \n", argv[1]);
		return 0;
	}

}
