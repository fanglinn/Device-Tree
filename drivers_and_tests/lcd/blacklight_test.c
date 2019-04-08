#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int main(int argc, char **argv)
{
    int turn;
    int fd;
    
    //检测命令后面带的参数
    if(argc == 1 || argc > 2)
    {
        printf("Usage: backlight_test on|off!\n");
        exit(1);
    }
    
    //打开背光设备
    fd = open("/dev/backlight", O_RDWR);
    
    if(fd < 0)
    {
        printf("Open Backlight Device Faild!\n");
        exit(1);
    }
    
    //判断输入的参数
    if(strcmp(argv[1], "on") == 0)
    {
        turn = 1;
    }
    else if(strcmp(argv[1], "off") == 0)
    {
        turn = 0;
    }
    else
    {
        printf("Usage: backlight_test on|off!\n");
        exit(1);
    }
    
    //进行IO控制
    ioctl(fd, turn);

    //关闭背光设备
    close(fd);

    return 0;
}












