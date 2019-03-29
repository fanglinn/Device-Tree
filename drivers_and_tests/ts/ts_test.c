#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>

struct sample{

	char position[15];

	int x;

	int y;

};

struct sample sample_array[4]=

{

	{"topleft",0,0},

	{"topright",0,0},

	{"bottonleft",0,0},

	{"bottonright",0,0},

};

int X1,X2,Y1,Y2;



getsample(int fd,int position)

{  
	struct input_event ev[128];

	int rb,sample_cnt,cntx=0,cnty=0;

	rb=read(fd,ev,sizeof(struct input_event)*128);



	if (rb < (int) sizeof(struct input_event)) {

		perror("evtest: short read");

		exit (1);

	}

	for (sample_cnt = 0;

		sample_cnt< (int) (rb / sizeof(struct input_event));

	sample_cnt++)

	{

		if (EV_ABS== ev[sample_cnt].type){
			if( sample_cnt%20==0){

				printf("%ld.%06ld ",

					ev[sample_cnt].time.tv_sec,

					ev[sample_cnt].time.tv_usec);

				printf("type %d code %d value %d\n",

					ev[sample_cnt].type,

					ev[sample_cnt].code, ev[sample_cnt].value);
			}

			if(ABS_X==ev[sample_cnt].code){

				sample_array[position].x+= ev[sample_cnt].value;

				cntx++;

			}

			if(ABS_Y==ev[sample_cnt].code){

				sample_array[position].y+= ev[sample_cnt].value;

				cnty++;

			}
		}

	}

	sample_array[position].x/=cntx;

	sample_array[position].y/=cnty;

}



int ts_coordinate(int value,int axes)
{
	int tempX,ret;
	if(ABS_X==axes)ret=240-(240*(value-X2)/(X1-X2));
	if(ABS_Y==axes)ret=320-(320*(value-Y2)/(Y1-Y2));
	return ret;
} 

int main(int argc, char **argv)

{

	struct pollfd pfd;

	int n,fd,i=0;

	if ((fd = open("/dev/input/event0",O_RDONLY) )< 0) {

		printf("open error! \n");

		exit(1);

	}


	for(i=0;i<4;i++){

		printf("Please touch the %s for 6 second ! \n",sample_array[i].position);

		sleep(6);

		printf("Time is up Please release\n");

		getsample(fd,i);
		sleep(1);

	}
	for(i=0;i<4;i++){
		printf("%12s x=%4d,y=%4d\n",sample_array[i].position,
			sample_array[i].x,
			sample_array[i].y);
	}
	X1=(sample_array[0].x+ sample_array[2].x )/2;  
	X2=(sample_array[1].x+ sample_array[3].x )/2;  
	Y1=(sample_array[0].y+ sample_array[1].y )/2;  
	Y2=(sample_array[2].y+ sample_array[3].y )/2;            

	printf("Coordinate complete,test it now\n");  
	for(i=0;i<4;i++){

		printf("Please touch the %s for 6 second ! \n",sample_array[i].position);

		sleep(6);

		printf("Time is up Please release\n");

		getsample(fd,i);
		sample_array[i].x=ts_coordinate(sample_array[i].x,ABS_X);
		sample_array[i].y=ts_coordinate(sample_array[i].y,ABS_Y);
		sleep(1);

	}
	printf("the data after coordinate \n");  
	for(i=0;i<4;i++){
		printf("%12s x=%4d,y=%4d\n",sample_array[i].position,
			sample_array[i].x,
			sample_array[i].y);
	}
	close(fd);
	exit(0);

}

