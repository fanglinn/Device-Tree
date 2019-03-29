// SPDX-License-Identifier: GPL-2.0
// linux/arch/arm/mach-s3c2440/mach-smdk2440.c
//
// Copyright (c) 2004-2005 Simtec Electronics
//	Ben Dooks <ben@simtec.co.uk>
//
// http://www.fluff.org/ben/smdk2440/
//
// Thanks to Dimity Andric and TomTom for the loan of an SMDK2440.

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/serial_s3c.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <mach/regs-gpio.h>
#include <mach/regs-lcd.h>

#include <mach/fb.h>
#include <linux/platform_data/i2c-s3c2410.h>

#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/samsung-time.h>

#include "common.h"
#include "common-smdk.h"

#include <linux/dm9000.h>

#define MACH_SMDK2440_DM9K_BASE (S3C2410_CS4 + 0x300)

static struct map_desc smdk2440_iodesc[] __initdata = {
	/* ISA IO Space map (memory space selected by A24) */

	{
		.virtual	= (u32)S3C24XX_VA_ISA_WORD,
		.pfn		= __phys_to_pfn(S3C2410_CS2),
		.length		= 0x10000,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (u32)S3C24XX_VA_ISA_WORD + 0x10000,
		.pfn		= __phys_to_pfn(S3C2410_CS2 + (1<<24)),
		.length		= SZ_4M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (u32)S3C24XX_VA_ISA_BYTE,
		.pfn		= __phys_to_pfn(S3C2410_CS2),
		.length		= 0x10000,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (u32)S3C24XX_VA_ISA_BYTE + 0x10000,
		.pfn		= __phys_to_pfn(S3C2410_CS2 + (1<<24)),
		.length		= SZ_4M,
		.type		= MT_DEVICE,
	}
};

#define UCON S3C2410_UCON_DEFAULT | S3C2410_UCON_UCLK
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE

static struct s3c2410_uartcfg smdk2440_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	/* IR port */
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = 0x3c5,
		.ulcon	     = 0x43,
		.ufcon	     = 0x51,
	}
};

/* LCD driver info */

static struct s3c2410fb_display smdk2440_lcd_cfg __initdata = {

	.lcdcon5	= S3C2410_LCDCON5_FRM565 |
			  S3C2410_LCDCON5_INVVLINE |
			  S3C2410_LCDCON5_INVVFRAME |
			  S3C2410_LCDCON5_PWREN |
			  S3C2410_LCDCON5_HWSWP,

	.type		= S3C2410_LCDCON1_TFT,

	.width		= 240,
	.height		= 320,

	.pixclock	= 166667, /* HCLK 60 MHz, divisor 10 */
	.xres		= 240,
	.yres		= 320,
	.bpp		= 16,
	.left_margin	= 20,
	.right_margin	= 8,
	.hsync_len	= 4,
	.upper_margin	= 8,
	.lower_margin	= 7,
	.vsync_len	= 4,
};

static struct s3c2410fb_mach_info smdk2440_fb_info __initdata = {
	.displays	= &smdk2440_lcd_cfg,
	.num_displays	= 1,
	.default_display = 0,

#if 0
	/* currently setup by downloader */
	.gpccon		= 0xaa940659,
	.gpccon_mask	= 0xffffffff,
	.gpcup		= 0x0000ffff,
	.gpcup_mask	= 0xffffffff,
	.gpdcon		= 0xaa84aaa0,
	.gpdcon_mask	= 0xffffffff,
	.gpdup		= 0x0000faff,
	.gpdup_mask	= 0xffffffff,
#endif

	.lpcsel		= ((0xCE6) & ~7) | 1<<4,
};

/* LCD driver info */

/*static struct s3c2410fb_display mini2440_lcd_cfg __initdata = {

	.lcdcon5	= S3C2410_LCDCON5_FRM565 |
			  S3C2410_LCDCON5_INVVLINE |
			  S3C2410_LCDCON5_INVVFRAME |
			  S3C2410_LCDCON5_PWREN |
			  S3C2410_LCDCON5_HWSWP,

	.type		= S3C2410_LCDCON1_TFT,

	.width		= 240,
	.height		= 320,

	.pixclock	= 166667, // HCLK 60 MHz, divisor 10 
	.xres		= 240,
	.yres		= 320,
	.bpp		= 16,
	.left_margin	= 20,
	.right_margin	= 8,
	.hsync_len	= 4,
	.upper_margin	= 8,
	.lower_margin	= 7,
	.vsync_len	= 4,
};

static struct s3c2410fb_mach_info mini2440_fb_info __initdata = {
	.displays	= &mini2440_lcd_cfg,
	.num_displays	= 1,
	.default_display = 0,

#if 0
	// currently setup by downloader 
	.gpccon		= 0xaa940659,
	.gpccon_mask	= 0xffffffff,
	.gpcup		= 0x0000ffff,
	.gpcup_mask	= 0xffffffff,
	.gpdcon		= 0xaa84aaa0,
	.gpdcon_mask	= 0xffffffff,
	.gpdup		= 0x0000faff,
	.gpdup_mask	= 0xffffffff,
#endif

	.lpcsel		= ((0xCE6) & ~7) | 1<<4,
};

*/

/* LCD driver info */

#if defined(CONFIG_FB_S3C2410_N240320)

#define LCD_WIDTH 240
#define LCD_HEIGHT 320
#define LCD_PIXCLOCK 100000

#define LCD_RIGHT_MARGIN 36
#define LCD_LEFT_MARGIN 19
#define LCD_HSYNC_LEN 5

#define LCD_UPPER_MARGIN 1
#define LCD_LOWER_MARGIN 5
#define LCD_VSYNC_LEN 1

#elif defined(CONFIG_FB_S3C2410_W320240)
#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#define LCD_PIXCLOCK 170000

#define LCD_RIGHT_MARGIN 0x44
#define LCD_LEFT_MARGIN 0x04
#define LCD_HSYNC_LEN 0x01

#define LCD_UPPER_MARGIN 10
#define LCD_LOWER_MARGIN 4
#define LCD_VSYNC_LEN 1

#define LCD_CON5 (S3C2410_LCDCON5_FRM565 | S3C2410_LCDCON5_INVVFRAME | S3C2410_LCDCON5_INVVLINE | S3C2410_LCDCON5_HWSWP )

#elif defined(CONFIG_FB_S3C2410_N480272)
#define LCD_WIDTH 480
#define LCD_HEIGHT 272
#define LCD_PIXCLOCK 100000

#define LCD_RIGHT_MARGIN 36
#define LCD_LEFT_MARGIN 19
#define LCD_HSYNC_LEN 5

#define LCD_UPPER_MARGIN 1
#define LCD_LOWER_MARGIN 5
#define LCD_VSYNC_LEN 1

#elif defined(CONFIG_FB_S3C2410_TFT640480)
#define LCD_WIDTH 640
#define LCD_HEIGHT 480
#define LCD_PIXCLOCK 40000

#define LCD_RIGHT_MARGIN 67
#define LCD_LEFT_MARGIN 40
#define LCD_HSYNC_LEN 31

#define LCD_UPPER_MARGIN 5
#define LCD_LOWER_MARGIN 25
#define LCD_VSYNC_LEN 1

#elif defined(CONFIG_FB_S3C2410_T240320)
#define LCD_WIDTH 240
#define LCD_HEIGHT 320
#define LCD_PIXCLOCK 170000
#define LCD_RIGHT_MARGIN 25
#define LCD_LEFT_MARGIN 0
#define LCD_HSYNC_LEN 4
#define LCD_UPPER_MARGIN 1
#define LCD_LOWER_MARGIN 4
#define LCD_VSYNC_LEN 1
#define LCD_CON5 (S3C2410_LCDCON5_FRM565 | S3C2410_LCDCON5_INVVDEN | S3C2410_LCDCON5_INVVFRAME | S3C2410_LCDCON5_INVVLINE | S3C2410_LCDCON5_INVVCLK | S3C2410_LCDCON5_HWSWP )

#elif defined(CONFIG_FB_S3C2410_X240320)
#define LCD_WIDTH 240
#define LCD_HEIGHT 320
#define LCD_PIXCLOCK 170000
#define LCD_RIGHT_MARGIN 25
#define LCD_LEFT_MARGIN 0
#define LCD_HSYNC_LEN 4
#define LCD_UPPER_MARGIN 0
#define LCD_LOWER_MARGIN 4
#define LCD_VSYNC_LEN 9
#define LCD_CON5 (S3C2410_LCDCON5_FRM565 | S3C2410_LCDCON5_INVVDEN | S3C2410_LCDCON5_INVVFRAME | S3C2410_LCDCON5_INVVLINE | S3C2410_LCDCON5_INVVCLK | S3C2410_LCDCON5_HWSWP )

#elif defined(CONFIG_FB_S3C2410_TFT800480)
#define LCD_WIDTH 800
#define LCD_HEIGHT 480
#define LCD_PIXCLOCK 40000

#define LCD_RIGHT_MARGIN 67
#define LCD_LEFT_MARGIN 40
#define LCD_HSYNC_LEN 31

#define LCD_UPPER_MARGIN 25
#define LCD_LOWER_MARGIN 5
#define LCD_VSYNC_LEN 1

#elif defined(CONFIG_FB_S3C2410_VGA1024768)
#define LCD_WIDTH 1024
#define LCD_HEIGHT 768
#define LCD_PIXCLOCK 80000

#define LCD_RIGHT_MARGIN 15
#define LCD_LEFT_MARGIN 199
#define LCD_HSYNC_LEN 15

#define LCD_UPPER_MARGIN 1
#define LCD_LOWER_MARGIN 1
#define LCD_VSYNC_LEN 1
#define LCD_CON5 (S3C2410_LCDCON5_FRM565 | S3C2410_LCDCON5_HWSWP)

#endif
#if defined (LCD_WIDTH)

static struct s3c2410fb_display mini2440_lcd_cfg __initdata = {
#if !defined (LCD_CON5)
	.lcdcon5        = S3C2410_LCDCON5_FRM565 |
		S3C2410_LCDCON5_INVVLINE |
		S3C2410_LCDCON5_INVVFRAME |
		S3C2410_LCDCON5_PWREN |
		S3C2410_LCDCON5_HWSWP,
#else
	.lcdcon5        = LCD_CON5,
#endif
	
	.type           = S3C2410_LCDCON1_TFT,
	
	.width          = LCD_WIDTH,
	.height         = LCD_HEIGHT,
	
	.pixclock       = LCD_PIXCLOCK,
	.xres           = LCD_WIDTH,
	.yres           = LCD_HEIGHT,
	.bpp            = 16,
	.left_margin    = LCD_LEFT_MARGIN + 1,
	.right_margin   = LCD_RIGHT_MARGIN + 1,
	.hsync_len      = LCD_HSYNC_LEN + 1,
	.upper_margin   = LCD_UPPER_MARGIN + 1,
	.lower_margin   = LCD_LOWER_MARGIN + 1,
	
	.vsync_len      = LCD_VSYNC_LEN + 1,
};

static struct s3c2410fb_mach_info mini2440_fb_info __initdata = {
	.displays       = &mini2440_lcd_cfg,
	.num_displays   = 1,
	.default_display = 0,
	
	.gpccon =       0xaa955699,
	.gpccon_mask =  0xffc003cc,
	.gpcup =        0x0000ffff,
	.gpcup_mask =   0xffffffff,
	
	.gpdcon =       0xaa95aaa1,
	.gpdcon_mask =  0xffc0fff0,
	.gpdup =        0x0000faff,
	.gpdup_mask =   0xffffffff,
	
	
	.lpcsel         = 0xf82,
};

#endif




/*
* DM9000 add by Flinn for mini2440
*/
static struct resource smdk2440_dm9k_resource[] = {
        [0] = DEFINE_RES_MEM(MACH_SMDK2440_DM9K_BASE, 4),
        [1] = DEFINE_RES_MEM(MACH_SMDK2440_DM9K_BASE + 4, 4),
        [2] = DEFINE_RES_NAMED(IRQ_EINT7, 1, NULL, IORESOURCE_IRQ \
                                                | IORESOURCE_IRQ_HIGHEDGE),
};

/*
 * The DM9000 has no eeprom, and it's MAC address is set by
 * the bootloader before starting the kernel.
 */
static struct dm9000_plat_data smdk2440_dm9k_pdata = {
        .flags          = (DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM),
};

static struct platform_device smdk2440_device_eth = {
        .name           = "dm9000",
        .id             = -1,
        .num_resources  = ARRAY_SIZE(smdk2440_dm9k_resource),
        .resource       = smdk2440_dm9k_resource,
        .dev            = {
                .platform_data  = &smdk2440_dm9k_pdata,
        },
};


static struct platform_device *smdk2440_devices[] __initdata = {
	&s3c_device_ohci,
	&s3c_device_lcd,
	&s3c_device_wdt,
	&s3c_device_i2c0,
	&s3c_device_iis,
	&smdk2440_device_eth,
};

static void __init smdk2440_map_io(void)
{
	s3c24xx_init_io(smdk2440_iodesc, ARRAY_SIZE(smdk2440_iodesc));
	s3c24xx_init_uarts(smdk2440_uartcfgs, ARRAY_SIZE(smdk2440_uartcfgs));
	samsung_set_timer_source(SAMSUNG_PWM3, SAMSUNG_PWM4);
}

static void __init smdk2440_init_time(void)
{
	//s3c2440_init_clocks(16934400);
	s3c2440_init_clocks(12000000);
	samsung_timer_init();
}

static void __init smdk2440_machine_init(void)
{
	//s3c24xx_fb_set_platdata(&smdk2440_fb_info);

#if defined (LCD_WIDTH)
	s3c24xx_fb_set_platdata(&mini2440_fb_info);
#endif

	s3c_i2c0_set_platdata(NULL);

	platform_add_devices(smdk2440_devices, ARRAY_SIZE(smdk2440_devices));
	smdk_machine_init();
}

/* add by Flinn */
static const char *const smdk2440_dt_compat[] __initconst = {
        "samsung,smdk2440", 
        NULL
};

MACHINE_START(S3C2440, "SMDK2440")
	/* Maintainer: Ben Dooks <ben-linux@fluff.org> */
	.atag_offset	= 0x100,
	.dt_compat      = smdk2440_dt_compat,   /* add by Flinn */

	.init_irq	= s3c2440_init_irq,
	.map_io		= smdk2440_map_io,
	.init_machine	= smdk2440_machine_init,
	.init_time	= smdk2440_init_time,
MACHINE_END
