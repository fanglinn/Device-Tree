#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <plat/regs-adc.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-samsung.h>
#include <plat/gpio-cfg.h>


/* For ts.dev.id.version */
#define S3C2410TSVERSION 0x0101

/*定义一个WAIT4INT宏，该宏将对ADC触摸屏控制寄存器进行操作
S3C2410_ADCTSC_YM_SEN这些宏都定义在regs-adc.h中*/
#define WAIT4INT(x)  (((x)<<8) | \
	S3C2410_ADCTSC_YM_SEN | S3C2410_ADCTSC_YP_SEN | S3C2410_ADCTSC_XP_SEN | \
	S3C2410_ADCTSC_XY_PST(3))

#define AUTOPST      (S3C2410_ADCTSC_YM_SEN | S3C2410_ADCTSC_YP_SEN | S3C2410_ADCTSC_XP_SEN | \
	S3C2410_ADCTSC_AUTO_PST | S3C2410_ADCTSC_XY_PST(0))

static char *s3c2410ts_name = "s3c2410 TouchScreen";
#define DEVICE_NAME   "mini2440_TouchScreen" /*设备名称*/

static struct input_dev *ts_dev; /*定义一个输入设备来表示我们的触摸屏设备*/

static long xp;
static long yp;
static int count;

/*定义一个外部的信号量ADC_LOCK，因为ADC_LOCK在ADC驱动程序中已申明
这样就能保证ADC资源在ADC驱动和触摸屏驱动中进行互斥访问*/
extern struct semaphore ADC_LOCK;
static int OwnADC = 0;

static void __iomem *base_addr; /*定义了一个用来保存经过虚拟映射后的内存地址*/

static inline void s3c2410_ts_connect(void)
{
	s3c_gpio_cfgpin(S3C2410_GPG(12), S3C2410_GPG12_XMON);
	s3c_gpio_cfgpin(S3C2410_GPG(13), S3C2410_GPG13_nXPON);
	s3c_gpio_cfgpin(S3C2410_GPG(14), S3C2410_GPG14_YMON);
	s3c_gpio_cfgpin(S3C2410_GPG(15), S3C2410_GPG15_nYPON);
}

static void touch_timer_fire(unsigned long data)
{
	/*用于记录这一次AD转换后的值*/
	unsigned long data0;
	unsigned long data1;
	int updown; /*用于记录触摸屏操作状态是按下还是抬起*/

	data0 = ioread32(base_addr+S3C2410_ADCDAT0);
	data1 = ioread32(base_addr+S3C2410_ADCDAT1);
	/*记录这一次对触摸屏是压下还是抬起，该状态保存在数据寄存器的第15位，所以需要逻辑与上S3C2410_ADCDAT0_UPDOWN*/
	updown = (!(data0 & S3C2410_ADCDAT0_UPDOWN)) && (!(data1 & S3C2410_ADCDAT0_UPDOWN));

	if (updown) /*判断触摸屏的操作状态*/
	{
		/*如果状态是按下，并且ADC已经转换了就报告事件和数据*/
		if (count != 0) //转换四次后进行事件汇报
		{ 
			long tmp;

			tmp = xp;
			xp = yp;
			yp = tmp;
			//这里进行转换是因为我们的屏幕使用时采用的是240*320，相当于把原来的屏幕的X,Y 轴变换。
			//个人理解，不知是否正确          

			//设备X,Y 值                                     
			xp >>= 2;
			yp >>= 2;
#ifdef CONFIG_TOUCHSCREEN_MINI2440_DEBUG
			/*触摸屏调试信息，编译内核时选上此项后，点击触摸屏会在终端上打印出坐标信息*/
			struct timeval tv;
			do_gettimeofday(&tv);
			printk(KERN_DEBUG "T: %06d, X: %03ld, Y: %03ld\n", (int)tv.tv_usec, xp, yp);
#endif
			input_report_abs(ts_dev, ABS_X, xp);
			input_report_abs(ts_dev, ABS_Y, yp);
			/*报告按键事件，键值为1(代表触摸屏对应的按键被按下)*/
			input_report_key(ts_dev, BTN_TOUCH, 1);
			/*报告触摸屏的状态，1表明触摸屏被按下*/
			input_report_abs(ts_dev, ABS_PRESSURE, 1);
			/*等待接收方受到数据后回复确认，用于同步*/
			input_sync(ts_dev);
			//这个表明我们上报了一次完整的触摸屏事件，用来间隔下一次的报告
		}
		/*如果状态是按下，并且ADC还没有开始转换就启动ADC进行转换*/
		xp = 0;
		yp = 0;
		count = 0;
		/*设置触摸屏的模式为自动转换模式*/
		iowrite32(S3C2410_ADCTSC_PULL_UP_DISABLE | AUTOPST, base_addr+S3C2410_ADCTSC);
		/*启动ADC转换*/
		iowrite32(ioread32(base_addr+S3C2410_ADCCON) | S3C2410_ADCCON_ENABLE_START, base_addr+S3C2410_ADCCON);
		//如果还没有启动ADC 或者ACD 转换四次完毕后则启动ADC
	} 
	else /*否则是抬起状态*/
	{
		//如果是up 状态，则提出报告并让触摸屏处在等待触摸的阶段
		count = 0;

		input_report_key(ts_dev, BTN_TOUCH, 0); /*报告按键事件，键值为0(代表触摸屏对应的按键被释放)*/
		input_report_abs(ts_dev, ABS_PRESSURE, 0); /*报告触摸屏的状态，0表明触摸屏没被按下*/
		input_sync(ts_dev); /*等待接收方受到数据后回复确认，用于同步*/

		iowrite32(WAIT4INT(0), base_addr+S3C2410_ADCTSC);
		if (OwnADC) 
		{
			OwnADC = 0;
			up(&ADC_LOCK);
		}
	}
}
/*定义并初始化了一个定时器touch_timer，定时器服务程序为touch_timer_fire*/
static struct timer_list touch_timer = __TIMER_INITIALIZER(touch_timer_fire, 0);

/*ADC中断服务程序，AD转换完成后触发执行*/
static irqreturn_t stylus_updown(int irq, void *dev_id)
{
	unsigned long data0;
	unsigned long data1;
	int updown;
	//注意在触摸屏驱动模块中，这个ADC_LOCK 的作用是保证任何时候都只有一个驱动程序使用ADC 的
	//中断线，因为在mini2440adc 模块中也会使用到ADC,这样只有拥有了这个锁，才能进入到启动ADC
	if (down_trylock(&ADC_LOCK) == 0) 
	{
		OwnADC = 1;
		data0 = ioread32(base_addr+S3C2410_ADCDAT0);
		data1 = ioread32(base_addr+S3C2410_ADCDAT1);
		/*记录这一次对触摸屏是压下还是抬起，该状态保存在数据寄存器的第15位，所以需要逻辑与上S3C2410_ADCDAT0_UPDOWN*/
		updown = (!(data0 & S3C2410_ADCDAT0_UPDOWN)) && (!(data1 & S3C2410_ADCDAT0_UPDOWN));

		if (updown) 
		{
			touch_timer_fire(0); //这是一个定时器函数，当然在这里是作为普通函数调用，用来启动ADC
		} 
		else 
		{
			OwnADC = 0;
			up(&ADC_LOCK); //注意这部分是基本不会执行的，除非你触摸后以飞快的速度是否，还来
			//不及启动ADC，当然这种飞快的速度一般是达不到的，笔者调试程序时发现这里是进入不了的
		}
	}

	return IRQ_HANDLED;
}


static irqreturn_t stylus_action(int irq, void *dev_id)
{
	unsigned long data0;
	unsigned long data1;

	if (OwnADC) { //读取数据
		data0 = ioread32(base_addr+S3C2410_ADCDAT0);
		data1 = ioread32(base_addr+S3C2410_ADCDAT1);

		xp += data0 & S3C2410_ADCDAT0_XPDATA_MASK;
		yp += data1 & S3C2410_ADCDAT1_YPDATA_MASK;
		count++;

		if (count < (1<<2)) { //如果小如四次重新启动转换
			iowrite32(S3C2410_ADCTSC_PULL_UP_DISABLE | AUTOPST, base_addr+S3C2410_ADCTSC);
			iowrite32(ioread32(base_addr+S3C2410_ADCCON) | S3C2410_ADCCON_ENABLE_START, base_addr+S3C2410_ADCCON);
		} else { //如果超过四次，则等待1ms 后进行数据上报
			mod_timer(&touch_timer, jiffies+1);
			iowrite32(WAIT4INT(1), base_addr+S3C2410_ADCTSC);
		}
	}

	return IRQ_HANDLED;
}

static struct clk *adc_clock; /*用于保存从平台时钟列表中获取的ADC时钟*/

static int __init s3c2410ts_init(void)
{
	struct input_dev *input_dev;
	/*从平台时钟队列中获取ADC的时钟，这里为什么要取得这个时钟，因为ADC的转换频率跟时钟有关。
	系统的一些时钟定义在arch/arm/plat-s3c24xx/s3c2410-clock.c中*/
	adc_clock = clk_get(NULL, "adc");
	if (!adc_clock) {
		printk(KERN_ERR "failed to get adc clock source\n");
		return -ENOENT;
	}
	/*时钟获取后要使能后才可以使用，clk_enable定义在arch/arm/plat-s3c/clock.c中*/
	clk_enable(adc_clock);
	//获取时钟，挂载APB BUS 上的外围设备，需要时钟控制，ADC 就是这样的设备。

	/*I/O 内存是不能直接进行访问的，必须对其进行映射，为I/O 内存分配虚拟地址，这些虚拟地址以__iomem
	进行说明，但不能直接对其进行访问，需要使用专用的函数，如iowrite32 
	S3C2410_PA_ADC是ADC控制器的基地址，定义在mach-s3c2410/include/mach/map.h中，0x20是虚拟地址长度大小*/
	base_addr=ioremap(S3C2410_PA_ADC,0x20);
	if (base_addr == NULL) {
		printk(KERN_ERR "Failed to remap register block\n");
		return -ENOMEM;
	}

	/* Configure GPIOs */
	s3c2410_ts_connect();
	/*计算结果为(二进制)：111111111000000，再根据数据手册得知此处是将AD转换预定标器值设为255、AD转换预定标器使能有效*/
	iowrite32(S3C2410_ADCCON_PRSCEN | S3C2410_ADCCON_PRSCVL(0xFF),\
		base_addr+S3C2410_ADCCON); //使能预分频和设置分频系数
	iowrite32(0xffff,  base_addr+S3C2410_ADCDLY); //设置ADC延时，在等待中断模式下表示产生 INT_TC 的间隔延时值为0xffff*/

	/*WAIT4INT宏计算结果为(二进制)：11010011，再根据数据手册得知此处是将ADC触摸屏控制寄存器设置成等待中断模式*/
	iowrite32(WAIT4INT(0), base_addr+S3C2410_ADCTSC); //按照等待中断的模式设置TSC


	/* Initialise input stuff */
	//allocate memory for new input device,用来给输入设备分配空间，并做一些输入设备通用的初始的设置
	input_dev = input_allocate_device();

	if (!input_dev) {
		printk(KERN_ERR "Unable to allocate the input device !!\n");
		return -ENOMEM;
	}
	//设置事件类型
	ts_dev = input_dev;
	ts_dev->evbit[0] = BIT(EV_SYN) | BIT(EV_KEY) | BIT(EV_ABS);
	ts_dev->keybit[BITS_TO_LONGS(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	input_set_abs_params(ts_dev, ABS_X, 0, 0x3FF, 0, 0);
	input_set_abs_params(ts_dev, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(ts_dev, ABS_PRESSURE, 0, 1, 0, 0);
	/*以上四句都是设置事件类型中的code，如何理解呢，先说明事件类型，常用的事件类型EV_KEY、
	EV_MOSSE, EV_ABS(用来接收像触摸屏这样的绝对坐标事件)，而每种事件又会有不同类型的编码code，
	比方说ABS_X，ABS_Y，这些编码又会有相应的value*/

	ts_dev->name = DEVICE_NAME;
	ts_dev->id.bustype = BUS_RS232;
	ts_dev->id.vendor = 0xDEAD;
	ts_dev->id.product = 0xBEEF;
	ts_dev->id.version = S3C2410TSVERSION;
	//以上是输入设备的名称和id，这些信息时输入设备的身份信息了，在用户空间如何看到呢?
	//可以通过cat /proc/bus/input/devices，下面是其输出信息
	/*[root@mini2440 /]#cat proc/bus/input/devices
	I: Bus=0013 Vendor=dead Product=beef Version=0101
	N: Name="s3c2410 TouchScreen"
	P: Phys=
	S: Sysfs=/devices/virtual/input/input0
	U: Uniq=
	H: Handlers=event0
	B: EV=b
	B: KEY=0
	B: ABS=1000003
	*/

	/* Get irqs */
	//中断处理
	//stylus_action 和stylus_updown 两个中断处理函数，当笔尖触摸时，会进入到stylus_updown
	if (request_irq(IRQ_ADC, stylus_action, IRQF_SHARED|0,
		"s3c2410_action", ts_dev)) {
			printk(KERN_ERR "s3c2410_ts.c: Could not allocate ts IRQ_ADC !\n");
			iounmap(base_addr);
			return -EIO;
	}
	if (request_irq(IRQ_TC, stylus_updown, 0,
		"s3c2410_action", ts_dev)) {
			printk(KERN_ERR "s3c2410_ts.c: Could not allocate ts IRQ_TC !\n");
			iounmap(base_addr);
			return -EIO;
	}

	printk(KERN_INFO "%s successfully loaded\n", s3c2410ts_name);

	/* All went ok, so register to the input system */
	//前面已经设置了设备的基本信息和所具备的能力，所有的都准备好了，现在就可以注册了
	input_register_device(ts_dev);

	return 0;
}

static void __exit s3c2410ts_exit(void)
{
	disable_irq(IRQ_ADC);
	disable_irq(IRQ_TC);
	free_irq(IRQ_TC,ts_dev);
	free_irq(IRQ_ADC,ts_dev);

	if (adc_clock) {
		clk_disable(adc_clock);
		clk_put(adc_clock);
		adc_clock = NULL;
	}

	input_unregister_device(ts_dev);
	iounmap(base_addr);
}
module_init(s3c2410ts_init);
module_exit(s3c2410ts_exit);

