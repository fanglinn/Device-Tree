#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
 
unsigned short img[320][240];
 
int main(void){
	int fd, i, j;
	printf("size of short:%d\n", sizeof(unsigned short));
 
	fd = open("/dev/fb1", O_RDWR);
	if(-1 == fd){
		printf("open device error!\n");
		return -1;
	}
 
	for(i=0; i<320; i++){
		for(j=0; j<240; j++){
			if(i<=100)
				img[i][j] = 0b1111100000000000;
			else if(i<=200)
				img[i][j] = 0b0000011111100000;
			else
				img[i][j] = 0b0000000000011111;
		}
	}
	write(fd, img[0], 320*248*sizeof(unsigned short));
	close(fd);
 
	return 0;
}

