
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

#include <asm/irq.h>
#include <asm/io.h>
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

static int led_pin;
static volatile unsigned int *gpio_con;
static volatile unsigned int *gpio_dat;

/* 123. 分配/设置/注册file_operations 
 * 4. 入口
 * 5. 出口
 */

static int major;
static struct class *led_class;

static dev_t dev;
static struct cdev *cdev;

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

static int led_open (struct inode *node, struct file *filp)
{
		printk(KERN_NOTICE "led drv open. \n");
	/* 把LED引脚配置为输出引脚 */
	/* GPF5 - 0x56000050 */
	int bank = led_pin >> 16;
	int base = gpio_base[bank];

	int pin = led_pin & 0xffff;
	gpio_con = ioremap(base, 8);
	if (gpio_con) {
		printk("ioremap(0x%x) = 0x%x\n", base, gpio_con);
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
	/* 根据APP传入的值来设置LED引脚 */
	unsigned char val;
	int pin = led_pin & 0xffff;
	
	copy_from_user(&val, buf, 1);

	if (val)
	{
		/* 点灯 */
		*gpio_dat &= ~(1<<pin);
	}
	else
	{
		/* 灭灯 */
		*gpio_dat |= (1<<pin);
	}

	return 1; /* 已写入1个数据 */
}

static int led_release (struct inode *node, struct file *filp)
{
	printk("iounmap(0x%x)\n", gpio_con);
	iounmap(gpio_con);
	return 0;
}


static struct file_operations myled_oprs = {
	.owner = THIS_MODULE,
	.open  = led_open,
	.write = led_write,
	.release = led_release,
};


static int led_probe(struct platform_device *pdev)
{
		printk(KERN_NOTICE "led drv probe. \n");
	struct resource		*res;

	/* 根据platform_device的资源进行ioremap */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	led_pin = res->start;

#if 0
	major = register_chrdev(0, "myled", &myled_oprs);

	led_class = class_create(THIS_MODULE, "myled");
	device_create(led_class, NULL, MKDEV(major, 0), NULL, "led"); /* /dev/led */
#endif

	int ret;

	if(major)
	{
		dev = MKDEV(major, 0);
		ret = register_chrdev_region(dev, 1, "led");
	}
	else
	{
		ret = alloc_chrdev_region(&dev, 0, 1, "led");
		major = MAJOR(dev);
	}

	cdev = cdev_alloc();
	if(!cdev)
	{
		unregister_chrdev_region(dev, 1);
		return -1;
	}
	cdev_init(cdev,  &myled_oprs);
	cdev->owner = THIS_MODULE;
	cdev->ops =  &myled_oprs;
	cdev_add(cdev, dev, 1);
	
	led_class = class_create(THIS_MODULE, "myled");
        device_create(led_class, NULL, MKDEV(major, 0), NULL, "led"); /* /dev/led */

	return 0;
}

static int led_remove(struct platform_device *pdev)
{
#if 1
	device_destroy(led_class,  MKDEV(major, 0));
	class_destroy(led_class);
#endif
	printk(KERN_NOTICE "led drv remove. \n");
	cdev_del(cdev);
	unregister_chrdev_region(dev, 1);
	
	return 0;
}


struct platform_driver led_drv = {
	.probe		= led_probe,
	.remove		= led_remove,
	.driver		= {
		.name	= "myled",
	}
};


static int myled_init(void)
{
	printk(KERN_NOTICE "led drv init. \n");
	platform_driver_register(&led_drv);
	return 0;
}

static void myled_exit(void)
{
		printk(KERN_NOTICE "led drv exit. \n");
	platform_driver_unregister(&led_drv);
}



module_init(myled_init);
module_exit(myled_exit);

MODULE_LICENSE("GPL");

