#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
        int fd;

        fprintf(stderr, "press Ctrl-C to stop\n");

        fd = open("/dev/adc", 0);

        if(fd < 0)
        {
                printf("Open ADC Device Faild!\n");
                exit(1);
        }

        while(1)
        {
                int ret;
                int data;

                fprintf(stderr, "read adc\n");
                ret = read(fd, &data, sizeof(data));

                fprintf(stderr, "read adc1\n");

                if(ret != sizeof(data))
                {
                        fprintf(stderr, "read adc2\n");

                        if(errno != EAGAIN)
                        {
                                printf("Read ADC Device Faild!\n");
                        }

                        continue;
                }
                else
                {
                        printf("Read ADC value is: %d\n", data);
}
                fprintf(stderr, "read adc3\n");

                 sleep(1);

        }

        close(fd);

        return 0;
}

