/*
 * linux-3.10.79
 * arm-linux-gcc-4.5.1
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/io.h>

#include <asm/div64.h>

#include <asm/mach/map.h>
#include <mach/regs-lcd.h>
#include <mach/regs-gpio.h>
#include <mach/fb.h>


struct fb_info *fbinfo;

static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpgcon;
static volatile unsigned long *gpgdat;
static volatile unsigned long *gpbcon;
static volatile unsigned long *gpbdat;
 
 
struct lcd_regs_addr{
	unsigned long lcdcon1;
	unsigned long lcdcon2;
	unsigned long lcdcon3;
	unsigned long lcdcon4;
	unsigned long lcdcon5;
	unsigned long lcdsaddr1;
	unsigned long lcdsaddr2;
	unsigned long lcdsaddr3;
	unsigned long redlut;
	unsigned long greenlut;
	unsigned long bluelut;
	unsigned long reserved[9];
	unsigned long dithmode;
	unsigned long tpal;
	unsigned long lcdintpnd;
	unsigned long lcdsrcpnd;
	unsigned long lcdintmsk;
	unsigned long lpcsel;
};


static volatile struct lcd_regs_addr *lcd_regs;
static u32 pseudo_palette[16]; //�ٵĵ�ɫ��


static inline unsigned int chan_to_field(unsigned int chan, const struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}
 
/* ���õ�ɫ��ĺ��� */
static int grh_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info)
{
	unsigned int val;
	if(regno >= 16){
		return 1;
	}
	val  = chan_to_field(red, &info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue, &info->var.blue);
	pseudo_palette[regno] = val;
	return 0;
}
 
static struct fb_ops lcd_ops = {
	.owner = THIS_MODULE,
	.fb_setcolreg = grh_lcdfb_setcolreg,
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
};



static int __init lcd_init(void)
{
	//����fb_info�ṹ��ʵ��
	fbinfo = framebuffer_alloc(0, NULL);
 
	printk(KERN_EMERG "lcd init function 1!\n");
 
	//����fb_info�ṹ�������
	strcpy(fbinfo->fix.id, "mini2440_lcd");
	fbinfo->fix.smem_len = 320*240*16/8; //�Դ泤��
	fbinfo->fix.type = FB_TYPE_PACKED_PIXELS;
	fbinfo->fix.visual = FB_VISUAL_TRUECOLOR; //�������ɫ
	fbinfo->fix.line_length = 240*2; //һ�е����ݳ���
 
	fbinfo->var.xres = 240; //���÷ֱ���
	fbinfo->var.yres = 320;
	fbinfo->var.xres_virtual = 240;
	fbinfo->var.yres_virtual = 320;
	fbinfo->var.bits_per_pixel = 16; //ÿ������16λ
 
	//RGB 5:6:5
	fbinfo->var.red.offset= 11;
	fbinfo->var.red.length=5;
	fbinfo->var.green.offset = 5;
	fbinfo->var.green.length = 6;
	fbinfo->var.blue.offset = 0; 
	fbinfo->var.green.length = 5;
	fbinfo->var.accel_flags = FB_ACTIVATE_NOW;
 
	
	fbinfo->fbops = &lcd_ops; //����lcd��صĲ�������
	fbinfo->screen_size = 240*320*2; //�Դ��С
 
	//����lcd��ص�����
	gpbcon = ioremap(0x56000010, 12);
	gpccon = ioremap(0x56000020, 12);
	gpdcon = ioremap(0x56000030, 12);
	gpgcon = ioremap(0x56000060, 12);
	gpgdat = gpgcon + 1;
	gpbdat = gpbcon + 1;
 
	*gpbcon &= ~(1<<2); //����gpb1ΪPWM����źţ����Ʊ�������
	*gpbcon |= (1<<3);
 
	*gpccon = 0xaaaaaaaa;
	*gpdcon = 0xaaaaaaaa;
 
	*gpgcon |= (3<<8);
	*gpgdat &= ~(1<<4); /*�رյ�Դ*/
	
 
	printk(KERN_EMERG"lcd init function 2!\n");
 
	//����lcd controler�Ĳ���
	lcd_regs = ioremap(0x4d000000, sizeof(struct lcd_regs_addr));
	/*
	lcdʱ���ź�Ƶ�ʣ�TFT��VCLK = 101.25M / [(CLKVAL + 1) �� 2] = 6.4M CLKVAL = 7
	bit[17:8] CLKVAL = 7
	bit[6:5] PNRMODE=0b11 (tft lcd)
	bit[4:1] BPPMODE=0b1100 (tft 16bpp)
	bit[0] ENDIV=0 lcdʹ���źţ�һ��ʼ�ȹر�ʹ��
	*/
	lcd_regs->lcdcon1 = (7<<8) | (3<<5) | (12<<1);
	/*
	ˮƽ�źŲ���
	bit[31:24] VBPD=1
	bit[23:14] LINEVAL=319
	bit[13:6] VFPD=1
	bit[5:0] VSPW=1
	*/
	lcd_regs->lcdcon2 = (1<<24) | (319<<14) | (1<<6) | (1<<0); 
	/*
	��ֱ�źŲ���
	bit[25:19] HBPD=19
	bit[18:8] HOZVAL=239
	bit[7:0] HFPD==9
	*/
	lcd_regs->lcdcon3 = (19<<19) | (239<<8) | (9<<0);
	/*
	��ֱ�źŲ���
	bit[7:0] HSPW=9
	*/
	lcd_regs->lcdcon4 = (9<<0);
	/*
	���ŵļ�������
	bit[11] FRM565=1
	bit[10] INVVCLK=1 ������ȡ����
	bit[9] INVVLINE=1 ˮƽͬ���źŷ���
	bit[8] INVVFRAME=1 ��ֱͬ���źŷ���
	bit[7] INVVD=0 �����źŲ���Ҫ����
	bit[6] INVVDEN=0 ����ʹ���źŲ���Ҫ����
	bit[5] INVPWREN=0 ��Դʹ���źŲ���Ҫ����
	bit[3] PWREN=0 �Ȳ�ʹ�õ�Դʹ���ź�
	bit[1:0] BSWP=0 HWSWP=1 �����ֽ����е�˳��
	*/
	lcd_regs->lcdcon5 = (1<<11) | (1<<10) | (1<<9) | (1<<8) | (0<<7) | (0<<6) | (0<<5) | (0<<3) | (0<<1) | (1<<0);
 
 
	printk(KERN_EMERG"lcd init function 3!\n");
 
	//�����Դ�,�����Դ�������ַ�������ַ
	fbinfo->screen_base = dma_alloc_writecombine(NULL, fbinfo->fix.smem_len, (dma_addr_t *)&(fbinfo->fix.smem_start), GFP_KERNEL);
 
 
	lcd_regs->lcdsaddr1 = (fbinfo->fix.smem_start>>1) & ~(3<<30);
	lcd_regs->lcdsaddr2 = ((fbinfo->fix.smem_start+fbinfo->fix.smem_len)>>1) & 0x1fffff;
	lcd_regs->lcdsaddr3 = 240; //һ�еĳ��ȣ���λ�ǰ���
 
 
	//����lcd
	lcd_regs->lcdcon1 |= (1<<0); //ʹ��lcd�ź�
	lcd_regs->lcdcon5 |= (1<<3); //ʹ��LCD_PWREN(GPG4)�ź�
	*gpgdat |= (1<<4); //����LCD_PWREN�ź�
	
 
	printk(KERN_EMERG"lcd init function 4!\n");
	//ע��fb_info�ṹ��ʵ��
	register_framebuffer(fbinfo);
	printk(KERN_EMERG"lcd init function 5!\n");
	
	return 0;
}

static void __exit lcd_exit(void)
{
	unregister_framebuffer(fbinfo);
	//�ر�lcd
	lcd_regs->lcdcon1 &= ~(1<<0);
	*gpgdat &= ~(1<<4); //�رձ���
	iounmap(gpccon);
	iounmap(gpdcon);
	iounmap(gpgcon);
	iounmap(gpbcon);
	dma_free_writecombine(NULL, fbinfo->fix.smem_len, fbinfo->screen_base, fbinfo->fix.smem_start);
	framebuffer_release(fbinfo);
	return;
}

module_init(lcd_init);
module_exit(lcd_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Flinn <Flinn682@foxmail.com>");
MODULE_DESCRIPTION("lcd driver for mini2440 base on linux-3.10.79");
