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
#include <linux/slab.h>      /* kmalloc()... */
#include <linux/cdev.h>

#include <linux/device.h>    /* class_device ... */
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>



static unsigned int gpio_base[] = {
	0x56000000, /* GPACON */
	0x56000010, /* GPBCON */
	0x56000020, /* GPCCON */
	0x56000030, /* GPDCON */
	0x56000040, /* GPECON */
	0x56000050, /* GPFCON */
	0x56000060, /* GPGCON */
	0x56000070, /* GPHCON */
	0,           /* GPICON */
	0x560000D0, /* GPJCON */
};


static volatile unsigned int *gpio_con;
static volatile unsigned int *gpio_dat;



struct led_device
{
	char *device_name;
	int led_pin;
	int major;
	struct cdev *cdev;
	struct class *led_class;
	dev_t led_dev;
};


static struct led_device *led_device;

static int led_open (struct inode *node, struct file *filp)
{
	printk(KERN_NOTICE "led drv open. \n");
	int bank = led_device->led_pin >> 16;
	int base = gpio_base[bank];

	int pin = led_device->led_pin & 0xffff;
	gpio_con = ioremap(base, 8);
	if (gpio_con) {
		printk(KERN_NOTICE "ioremap(0x%x) = 0x%x\n", base, gpio_con);
	}
	else {
		return -EINVAL;
	}
	
	gpio_dat = gpio_con + 1;

	*gpio_con &= ~(3<<(pin * 2));
	*gpio_con |= (1<<(pin * 2));  

	return 0;
}

static ssize_t led_write (struct file *filp, const char __user *buf, size_t size, loff_t *off)
{
	unsigned char val;
	int pin = led_device->led_pin & 0xffff;
	
	copy_from_user(&val, buf, 1);

	if (val)
	{
		/* open */
		*gpio_dat &= ~(1<<pin);
	}
	else
	{
		/* close */
		*gpio_dat |= (1<<pin);
	}

	return 1;
}

static int led_release (struct inode *node, struct file *filp)
{
	printk(KERN_NOTICE "led release \n");
	iounmap(gpio_con);
	return 0;
}


static struct file_operations myled_ops = {
	.owner = THIS_MODULE,
	.open  = led_open,
	.write = led_write,
	.release = led_release,
};


static int led_drv_probe(struct platform_device *pdevice)
{
	struct resource *res;
	int ret;

	printk(KERN_NOTICE "led drv probe. \n");

	led_device = kmalloc(sizeof(struct led_device), GFP_KERNEL);
	if(!led_device)
	{
		printk(KERN_ERR "kmalloc fail .\n");
		return -ENOMEM;
	}

	strcpy(led_device->device_name, "myled");
	

	res = platform_get_resource(pdevice,IORESOURCE_MEM, 0);
	led_device->led_pin = res->start;

	if(led_device->major)
	{
		led_device->led_dev = MKDEV(led_device->major, 0);
		ret = register_chrdev_region(led_device->led_dev, 1, led_device->device_name);
	}
	else
	{
		ret = alloc_chrdev_region(&led_device->led_dev, 0, 1, led_device->device_name);
		led_device->major = MAJOR(led_device->led_dev);
	}

	led_device->cdev = cdev_alloc();
	if(!led_device->cdev)
	{
		kfree(led_device);
		unregister_chrdev_region(led_device->led_dev, 1);
		return -1;
	}
	cdev_init(led_device->cdev,  &myled_ops);
	led_device->cdev->owner = THIS_MODULE;
	led_device->cdev->ops =  &myled_ops;
	cdev_add(led_device->cdev, led_device->led_dev, 1);

	led_device->led_class = class_create(THIS_MODULE, "myled");

	device_create(led_device->led_class, NULL, led_device->led_dev, NULL, "myled");
	
	return 0;
}

static int led_drv_release(struct platform_device *device)
{
	printk(KERN_NOTICE "led drv release. \n");

	device_destroy(led_device->led_class,  led_device->led_dev);
	class_destroy(led_device->led_class);
	
	cdev_del(led_device->cdev);
	unregister_chrdev_region(led_device->led_dev, 1);
	kfree(led_device);
	
	return 0;
}


static struct platform_driver led_plat_drv = 
{
	.probe = led_drv_probe,
	.remove = led_drv_release,
	.driver = 
	{
		.name = "myled",
	},
};



static int __init led_drv_init(void)
{
	printk(KERN_NOTICE "led drv init. \n");
	platform_driver_register(&led_plat_drv);
	return 0;
}

static void __exit led_drv_exit(void)
{
	printk(KERN_NOTICE "led drv exit. \n");
	platform_driver_unregister(&led_plat_drv);
}

module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Flinn <Flinn682@foxmail.com>");

MODULE_DESCRIPTION("led driver for mini2440 base on linux-3.10.79");

