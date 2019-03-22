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
#include <asm/uaccess.h>
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


static unsigned int gpio_base[] = {
	0x56000000, /* GPACON */
	0x56000010, /* GPBCON */
	0x56000020, /* GPCCON */
	0x56000030, /* GPDCON */
	0x56000040, /* GPECON */
	0x56000050, /* GPFCON */
	0x56000060, /* GPGCON */
	0x56000070, /* GPHCON */
	0,          /* GPICON */
	0x560000D0, /* GPJCON */
};


static int led_pin;
static volatile unsigned int *gpio_con;
static volatile unsigned int *gpio_dat;


struct led_device
{
	int major;
	char device_name[32];
	int led_pin;
	dev_t led_dev;
	struct cdev *cdev;
	struct class *led_class;
};

static struct led_device *led_device = NULL;



static int led_open (struct inode *inode, struct file *filp)
{
	printk(KERN_NOTICE "led open .\n");
	int bank = led_pin >> 16;
	int base = gpio_base[bank];

	int pin = led_pin & 0xffff;
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

static int led_close (struct inode *inode, struct file *filp)
{
	printk(KERN_NOTICE "iounmap(0x%x)\n", gpio_con);
	iounmap(gpio_con);
	return 0;
}

static ssize_t led_write (struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	int val[8];
	int cmd, ops;

	printk(KERN_NOTICE "led write .\n");

	if (copy_from_user(&val, buf, 8))
			return -EFAULT;

	cmd = val[0];  
	ops = val[1];

	if(cmd == 0)    // close
	{
		switch(ops)
		{
			case 0:
				*gpio_dat |= (1 << 5);
				break;

			case 1:
				*gpio_dat |= (1 << 6);
				break;

			case 2:
				*gpio_dat |= (1 << 7);
				break;

			case 3:
				*gpio_dat |= (1 << 8);
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
				*gpio_dat &= ~(1 << 5);	
				break;

			case 1:
				*gpio_dat &= ~(1 << 6);		
				break;

			case 2:
				*gpio_dat &= ~(1 << 7);		
				break;

			case 3:
				*gpio_dat &= ~(1 << 8);		
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

const struct file_operations led_fops = 
{
	.owner  = THIS_MODULE,
	.open   = led_open,
	.release = led_close,
	.write  = led_write,
};


static int led_dt_probe(struct platform_device *platdata)
{
	int ret;
	struct resource		*res;

	printk(KERN_NOTICE "led dt probe .\n");
	
	led_device = kmalloc(sizeof(struct led_device), GFP_KERNEL);
	if(!led_device)
	{
		printk(KERN_ERR "led device kmalloc fail .\n");
		return -ENOMEM;
	}

	strcpy(led_device->device_name , "myled");

	/*
	* get resource from dtb
	*/
	res = platform_get_resource(platdata, IORESOURCE_MEM, 0);
	if(res){
		led_pin = res->start;
	}
	else{
		of_property_read_u32(platdata->dev.of_node, "pin", &led_pin);
	}

	if (!led_pin) 
	{
		printk("can not get pin for led\n");
		return -EINVAL;
	}

	if(led_device->major)
	{
		led_device->led_dev = MKDEV(led_device->major, 0);
		ret = register_chrdev_region(led_device->led_dev, 1, led_device->device_name);
	}
	else
	{
		alloc_chrdev_region(&led_device->led_dev, 0, 1, led_device->device_name);
		ret = led_device->major = MAJOR(led_device->led_dev);
	}

	if(ret < 0)
	{
		kfree(led_device);
		return -1;
	}

	led_device->cdev = cdev_alloc();
	if(!led_device->cdev)
	{
		printk(KERN_ERR "cdev alloc fail .\n");
		unregister_chrdev_region(led_device->led_dev, 1);
		kfree(led_device);
		return -1;
	}
	cdev_init(led_device->cdev, &led_fops);
	led_device->cdev->owner = THIS_MODULE;
	led_device->cdev->ops = &led_fops;
	cdev_add(led_device->cdev, led_device->led_dev, 1);

	led_device->led_class = class_create(THIS_MODULE, "myled");
	device_create(led_device->led_class,NULL, led_device->led_dev, NULL, "myled");

	return 0;
}

static int led_dt_remove(struct platform_device *platdata)
{
	printk(KERN_NOTICE "led dt remove .\n");
	device_destroy(led_device->led_class, led_device->led_dev);
	class_destroy(led_device->led_class);
	cdev_del(led_device->cdev);
	unregister_chrdev_region(led_device->led_dev, 1);
	kfree(led_device);

	return 0;
}


static const struct of_device_id of_match_leds[] = {
	{ .compatible = "mini2440_led", .data = NULL },
	{ /* sentinel */ }
};


static struct  platform_driver led_plat_driver = 
{
	.probe = led_dt_probe,
	.remove = led_dt_remove,
	.driver = 
	{
		.name = "mini2440_led",
		.of_match_table = of_match_leds,
	},
};

static int __init led_dt_drv_init(void)
{
	printk(KERN_NOTICE "led dt init .\n");
	
	platform_driver_register(&led_plat_driver);
	return 0;
}

static void __exit led_dt_drv_exit(void)
{
	printk(KERN_NOTICE "led exit .\n");

	platform_driver_unregister(&led_plat_driver);
}


module_init(led_dt_drv_init);
module_exit(led_dt_drv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Flinn <Flinn682@foxmail.com>");

MODULE_DESCRIPTION("led driver for mini2440 base on linux-3.10.79");

