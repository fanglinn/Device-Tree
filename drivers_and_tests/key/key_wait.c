/*
 * linux-3.10.79
 * arm-linux-gcc-4.5.1
*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>

#include <asm/irq.h>
#include <asm/io.h>
#include <linux/slab.h>      /* kmalloc(),... */
#include <linux/cdev.h>


#include <linux/device.h>    /* class_device ... */

#include <asm/gpio.h>

#include <linux/gpio.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>


#include <mach/gpio-samsung.h>

struct key_device
{
	int major;
	dev_t dev;
	struct cdev *cdev;
	char device_name[32];
	struct class *key_class;
	int key_event;
};


static struct key_device *key_dev = NULL;

/**
*  buttons of mini2440, low triggered
*    K1     GPG0    EINT8
*    K2     GPG3    EINT11
*    K3     GPG5    EINT13
*    K4     GPG6    EINT14
*    K5     GPG7    EINT15
*    K6     GPG11   EINT19
*/

struct key_desc
{
	int irq;
	unsigned int pin;
	unsigned long flags;
	unsigned int key_val;
	char *name;
};

static DECLARE_WAIT_QUEUE_HEAD(key_waitq);
static int press_flag = 0;


static const struct key_desc key_desc[] = 
{
	{IRQ_EINT8 , S3C2410_GPG(0) , IRQF_TRIGGER_FALLING , 0x1,  "key1"},
	{IRQ_EINT11, S3C2410_GPG(3) , IRQF_TRIGGER_FALLING , 0x2,  "key2"},
	{IRQ_EINT13, S3C2410_GPG(5) , IRQF_TRIGGER_FALLING , 0x3,  "key3"},
	{IRQ_EINT14, S3C2410_GPG(6) , IRQF_TRIGGER_FALLING , 0x4,  "key4"},
	{IRQ_EINT15, S3C2410_GPG(7) , IRQF_TRIGGER_FALLING , 0x5,  "key5"},
	{IRQ_EINT19, S3C2410_GPG(11), IRQF_TRIGGER_FALLING , 0x6,  "key6"},
};

static unsigned int key_value = 0;

irqreturn_t key_irq_handler(int irq, void *dev_id)
{
	struct key_desc *p = dev_id;
	int value ;
	
	wake_up_interruptible(&key_waitq);

	value = gpio_get_value(p->pin);

	if(value)
	{
		/* press up */
		key_value = p->key_val;
	}
	else
	{
		key_value = p->key_val + 0x80;
	}

	press_flag = 1;
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static ssize_t key_wait_read (struct file *pfile, char __user *usrbuf, size_t len, loff_t *offset)
{
	wait_event_interruptible(key_waitq, press_flag);

	if(copy_to_user(usrbuf, &key_value, 1))
		return -EFAULT;

	press_flag = 0;
	
	return 0;
}

static int key_wait_open (struct inode *inode, struct file *pfile)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(key_desc);i++)
	{
		if(request_irq(key_desc[i].irq, key_irq_handler, key_desc[i].flags, key_desc[i].name, (void *)&key_desc[i]))
			break;
	}
	
	return 0;
}

static int key_wait_close (struct inode *inode, struct file *pfile)
{
	int i;

	for(i = ARRAY_SIZE(key_desc) - 1; i >= 0; i++)
	{
		free_irq(key_desc[i].irq, (void *)&key_desc[i]);
	}
	return 0;
}

static const struct file_operations key_fops = 
{
	.owner = THIS_MODULE,
	.open = key_wait_open,
	.read = key_wait_read,
	.release = key_wait_close,
};

static int __init key_wait_init(void)
{
	int ret ;

	printk(KERN_NOTICE "init .\n");

	key_dev = kmalloc(sizeof(struct key_device), GFP_KERNEL);
	if(!key_dev)
	{
		return -ENOMEM;
	}

	strcpy(key_dev->device_name, "mykey");

	if(key_dev->major)
	{
		key_dev->dev = MKDEV(key_dev->major, 0);
		ret = register_chrdev_region(key_dev->dev, 1, key_dev->device_name);
	}
	else
	{
		ret = alloc_chrdev_region(&key_dev->dev, 1, 1, key_dev->device_name);
		key_dev->major = MAJOR(key_dev->dev);
	}

	if (ret < 0)
	{
		kfree(key_dev);
		return -EFAULT;
	}

	key_dev->cdev = cdev_alloc();
	if(!key_dev->cdev)
	{
		unregister_chrdev_region(key_dev->dev, 1);
		kfree(key_dev);
		return -EFAULT;
	}
	cdev_init(key_dev->cdev, &key_fops);
	key_dev->cdev->owner = THIS_MODULE;
	key_dev->cdev->ops = &key_fops;
	cdev_add(key_dev->cdev, key_dev->dev, 1);

	key_dev->key_class = class_create(THIS_MODULE, key_dev->device_name);

	device_create(key_dev->key_class, NULL, key_dev->dev, NULL, key_dev->device_name);

	
		
	return 0;
}

static void __exit key_wait_exit(void)
{
	printk(KERN_NOTICE "exit .\n");
	
	device_destroy(key_dev->key_class, key_dev->dev);

	class_destroy(key_dev->key_class);

	cdev_del(key_dev->cdev);
	unregister_chrdev_region(key_dev->dev, 1);
	kfree(key_dev);
}



module_init(key_wait_init);
module_exit(key_wait_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Flinn <Flinn682@foxmail.com>");

MODULE_DESCRIPTION("key driver for mini2440 base on linux-3.10.79");