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

/**
* mini2440 LED资源   低电平有效
*
* LED1    GPB5  
* LED2    GPB6
* LED3    GPB7
* LED4    GPB8
*/



#define GPB_ADDR			0x56000010


static volatile unsigned int *gpbcon;
static volatile unsigned int *gpbdat;


struct led_device
{
	int major;
	struct cdev *cdev;
	struct class *led_class;
	dev_t led_dev;
};

const char *device_name = "myled";
static struct led_device *led_device;

static int leds_open (struct inode *pinode, struct file *pfile)
{
	*gpbcon |= (0x1 << (5*2)) | (0x1 << (6*2)) | (0x1 << (7*2)) | (0x1 << (8*2));
	return 0;
}

static int leds_release (struct inode *pinode, struct file *pfile)
{
	return 0;
}


static ssize_t leds_write (struct file *pfile, const char __user *usrbuf, size_t len, loff_t *offset)
{
	int val[8];
	int cmd, ops;

	if (copy_from_user(&val, usrbuf, 8))
			return -EFAULT;

	cmd = val[0];  
	ops = val[1];

	if(cmd == 0)    // close
	{
		switch(ops)
		{
			case 0:
				*gpbdat |= (1 << 5);
				break;

			case 1:
				*gpbdat |= (1 << 6);
				break;

			case 2:
				*gpbdat |= (1 << 7);
				break;

			case 3:
				*gpbdat |= (1 << 8);
				break;

			default:
				break;
		}
	}
	else if(cmd == 1)  // open
	{
		switch(ops)
		{
			case 0:
				*gpbdat &= ~(1 << 5);	
				break;

			case 1:
				*gpbdat &= ~(1 << 6);		
				break;

			case 2:
				*gpbdat &= ~(1 << 7);		
				break;

			case 3:
				*gpbdat &= ~(1 << 8);		
				break;

			default:
				break;
		}
	}
	else
	{
		// do nothing .
	}
	return 0;
}

static const struct file_operations leds_fop = {
	.owner = THIS_MODULE,
	.open = leds_open,
	.write = leds_write,
	.release = leds_release,
};

static int __init led_drv_init(void)
{
	int ret;
	
	led_device = kmalloc(sizeof(struct led_device), GFP_KERNEL);
	if(!led_device)
	{
		printk(KERN_ERR "kmalloc err .\r");
		return -EBUSY;
	}

	/*
	*  register chrdev
	*/
	if(led_device->major)
	{
		led_device->led_dev = MKDEV(led_device->major, 0);
		ret = register_chrdev_region(led_device->led_dev, 1,device_name);
	}
	else
	{
		alloc_chrdev_region(&led_device->led_dev, 1, 1, device_name);
		ret = led_device->major = MAJOR(led_device->led_dev);
	}

	if(ret < 0)
	{
		printk(KERN_ERR "chrdev register error .\n");
		kfree(led_device);
		return -1;
	}

	led_device->cdev = cdev_alloc();
	if(!led_device->cdev)
	{
		printk(KERN_ERR "cdev alloc fail .\n");
		unregister_chrdev_region(led_device->led_dev, 1);
		kfree(led_device);
	}

	cdev_init(led_device->cdev, &leds_fop);
	led_device->cdev->owner = THIS_MODULE;
	led_device->cdev->ops = &leds_fop;
	cdev_add(led_device->cdev, led_device->led_dev, 1);

	led_device->led_class = class_create(THIS_MODULE, device_name);

	device_create(led_device->led_class, NULL, led_device->led_dev, NULL, device_name);

	/*
	* io remap
	*/
	gpbcon = (volatile unsigned int*)ioremap(GPB_ADDR, 4);
	gpbdat = gpbcon + 1;

	return 0;
}

static void __exit led_drv_exit(void)
{
	iounmap(gpbcon);

	device_destroy(led_device->led_class,  led_device->led_dev);
	class_destroy(led_device->led_class);
	cdev_del(led_device->cdev);
	
	unregister_chrdev_region(led_device->led_dev, 1);
	kfree(led_device);
}


module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Flinn <Flinn682@foxmail.com>");

MODULE_DESCRIPTION("led driver for mini2440 base on linux-3.10.79");

