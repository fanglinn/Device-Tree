#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


int main(int argc, char **argv)
{
	int fd ;
	unsigned char key_vals = 0;

	fd = open("/dev/mykey", O_RDWR);
	if(fd < 0)
	{
		printf("open fail. \n");
		return -1;
	}

	while (1)
	{
		read(fd, &key_vals, 1);
		printf("key_val = 0x%x\n", key_vals);
	}

	close(fd);
	return 0;
}


