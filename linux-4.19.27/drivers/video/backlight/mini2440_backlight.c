#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <mach/regs-clock.h>
#include <mach/gpio-samsung.h>
#include <mach/regs-gpio.h>
#include <linux/cdev.h>

#define DEVICE_NAME "backlight"
#define DEVICE_MINOR 5

static long mini2440_backlight_ioctl(
                struct file *file,
                unsigned int cmd,
                unsigned long arg)
{
        switch(cmd)
        {
                case 0:

                        gpio_set_value(S3C2410_GPG(4), 0);
                        printk(DEVICE_NAME " turn off!\n");
                        return 0;
                case 1:

                        gpio_set_value(S3C2410_GPG(4), 1);
                        printk(DEVICE_NAME " turn on!\n");
                        return 0;
                default:
                        return -EINVAL;
        }
}
static struct file_operations dev_fops =
{
        .owner = THIS_MODULE,
        .unlocked_ioctl = mini2440_backlight_ioctl,
};

static struct miscdevice misc =
{
        .minor = DEVICE_MINOR,
        .name = DEVICE_NAME,
        .fops = &dev_fops,
};

static int __init dev_init(void)
{
        int ret;
        ret = misc_register(&misc);
        if(ret < 0)
        {
                printk("Register misc device fiald!");
                return ret;
        }
        gpio_direction_output(S3C2410_GPG(4), 1);
        return ret;
}

static void __exit dev_exit(void)
{
        misc_deregister(&misc);
}

module_init(dev_init);
module_exit(dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ubuntu");
MODULE_DESCRIPTION("Backlight control for mini2440");

