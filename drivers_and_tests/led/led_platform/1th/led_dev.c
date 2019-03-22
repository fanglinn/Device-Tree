/*
 * linux-3.10.79
 * arm-linux-gcc-4.5.1
*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <linux/cdev.h>

#include <linux/device.h>    /* class_device ... */
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>


#define S3C2440_GPA(n)  (0<<16 | n)
#define S3C2440_GPB(n)  (1<<16 | n)
#define S3C2440_GPC(n)  (2<<16 | n)
#define S3C2440_GPD(n)  (3<<16 | n)
#define S3C2440_GPE(n)  (4<<16 | n)
#define S3C2440_GPF(n)  (5<<16 | n)
#define S3C2440_GPG(n)  (6<<16 | n)
#define S3C2440_GPH(n)  (7<<16 | n)
#define S3C2440_GPI(n)  (8<<16 | n)
#define S3C2440_GPJ(n)  (9<<16 | n)

static struct resource led_drv_res[] = 
{ 
	[0] = {
		.start = S3C2440_GPB(5),
		.end = S3C2440_GPB(5),
		.flags = IORESOURCE_MEM,
	},	
};

static void led_release(struct device * dev)
{
	printk(KERN_NOTICE "led dev release. \n");
}


static struct platform_device led_dev_plat = 
{
	.name = "myled",
	.id = -1,
	.num_resources = ARRAY_SIZE(led_drv_res),
	.resource = &led_drv_res,
	.dev = {
		.release = led_release
		},
};


static int __init led_dev_init(void)
{
	printk(KERN_NOTICE "led dev init. \n");

	platform_device_register(&led_dev_plat);
	return 0;
}

static void __exit led_dev_exit(void)
{
	printk(KERN_NOTICE "led dev exit. \n");
	platform_device_unregister(&led_dev_plat);
}

module_init(led_dev_init);
module_exit(led_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Flinn <Flinn682@foxmail.com>");

MODULE_DESCRIPTION("led driver for mini2440 base on linux-3.10.79");

