#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/uaccess.h>
#include <mach/regs-clock.h>
#include <plat/regs-adc.h>
#include <mach/regs-gpio.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>

#include "s3c24xx-adc.h"

#undef DEBUG
#ifdef DEBUG
#define DPRINTK(x...) {printk("%s %d\n",__FUNCTION__,__LINE__);}
#else
#define DPRINTK(x...) (void)(0)
#endif

#define DEVICE_NAME "adc"

static void __iomem *adc_base; 

typedef struct {
	 wait_queue_head_t wait;
	  int channel;
	   int prescale;
}ADC_DEV;
static ADC_DEV adcdev;

DEFINE_SEMAPHORE(ADC_LOCK);


static volatile int ev_adc = 0;


static int adc_data;


static struct clk *adc_clk;

#define ADCCON (*(volatile unsigned long *)(adc_base + S3C2410_ADCCON)) 
#define ADCTSC (*(volatile unsigned long *)(adc_base + S3C2410_ADCTSC)) 
#define ADCDLY (*(volatile unsigned long *)(adc_base + S3C2410_ADCDLY)) 
#define ADCDAT0 (*(volatile unsigned long *)(adc_base + S3C2410_ADCDAT0)) 
#define ADCDAT1 (*(volatile unsigned long *)(adc_base + S3C2410_ADCDAT1)) 
#define ADCUPDN (*(volatile unsigned long *)(adc_base + 0x14)) 
#define PRESCALE_DIS (0 << 14)
#define PRESCALE_EN (1 << 14)
#define PRSCVL(x) ((x) << 6)
#define ADC_INPUT(x) ((x) << 3)
#define ADC_START (1 << 0)
#define ADC_ENDCVT (1 << 15)

#define start_adc(ch, prescale) \
	do{ \
		 ADCCON = PRESCALE_EN | PRSCVL(prescale) | ADC_INPUT((ch)) ; \
		 ADCCON |= ADC_START; \
	}while(0)

static irqreturn_t adc_irq(int irq, void *dev_id)
{
	if (!ev_adc) 
	{
	    adc_data = ADCDAT0 & 0x3ff;
       	ev_adc = 1;
	    
	    wake_up_interruptible(&adcdev.wait);
	}
	return IRQ_HANDLED;
}

static ssize_t adc_read(struct file *filp, char *buffer, size_t count, loff_t *ppos)
{

	 if (down_trylock(&ADC_LOCK)) 
	 {
		 return -EBUSY;
	 }
	 if(!ev_adc) 
	 {
		 if(filp->f_flags & O_NONBLOCK)
		 {
			 return -EAGAIN;
		 }
		 else
		 {
			 start_adc(adcdev.channel, adcdev.prescale);
			 wait_event_interruptible(adcdev.wait, ev_adc);
		 }
	 }
	 ev_adc = 0;
	 
	 copy_to_user(buffer, (char *)&adc_data, sizeof(adc_data));
	 
	 up(&ADC_LOCK);
	 
	 return sizeof(adc_data);
	 
}

static int adc_open(struct inode *inode, struct file *filp)
{
	int ret; 
	/* normal ADC */
	ADCTSC = 0;

	init_waitqueue_head(&(adcdev.wait));
	adcdev.channel=0;
	adcdev.prescale=0xff;
	ret = request_irq(IRQ_ADC, adc_irq, IRQF_SHARED, DEVICE_NAME, &adcdev);
	if (ret) 
	{
		printk(KERN_ERR "IRQ%d error %d\n", IRQ_ADC, ret);
		return -EINVAL;
	}
	
	DPRINTK( "adc opened\n");
	return 0;
	
}

static int adc_release(struct inode *inode, struct file *filp)
{
	 DPRINTK( "adc closed\n");
	  return 0;
}

static struct file_operations dev_fops = {
	owner: THIS_MODULE,
	 open: adc_open,
      	 read: adc_read,
      release: adc_release,
};

static struct miscdevice adc_miscdev = {
	 .minor = MISC_DYNAMIC_MINOR,
	 .name = DEVICE_NAME,
	 .fops = &dev_fops,
};

static int __init dev_init(void)
{
	int ret;
	adc_clk = clk_get(NULL, "adc");
	
	clk_prepare_enable(adc_clk);
	
	if (!adc_clk) {
		printk(KERN_ERR "failed to get adc clock source\n");
		return -ENOENT;
	}
	

	clk_enable(adc_clk);


	adc_base=ioremap(S3C2410_PA_ADC,0x20);
	if (adc_base == NULL) {
		printk(KERN_ERR "Failed to remap register block\n");
		ret = -EINVAL;
		goto err_noclk;
	}


	ret = misc_register(&adc_miscdev);
	if (ret) 			        
       	{

		printk(KERN_ERR "Cannot register miscdev on minor=%d (%d)\n", MISC_DYNAMIC_MINOR, ret);
		goto err_nomap;
	}
	
	printk(DEVICE_NAME "\tinitialized!\n");
	
	return 0;
	
err_noclk:
	clk_disable(adc_clk);
	clk_put(adc_clk);

err_nomap:
	iounmap(adc_base);
	
	return ret;
}

static void __exit dev_exit(void)
{	
	free_irq(IRQ_ADC, &adcdev); 
	iounmap(adc_base);
	if (adc_clk)  
	{
		clk_disable(adc_clk);
		clk_put(adc_clk);
		adc_clk = NULL;
	}
	misc_deregister(&adc_miscdev);
}

EXPORT_SYMBOL(ADC_LOCK);

module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ubuntu");
MODULE_DESCRIPTION("Mini2440 ADC Driver");

