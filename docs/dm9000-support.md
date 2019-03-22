for mini2440
this file is support for linux-3.10.79 and linux-4.19.27

arm-linux-gcc-4.9.4


vim arch/arm/mach-s3c24xx/mach-smdk2440.c

#include <linux/dm9000.h>
...
#define MACH_SMDK2440_DM9K_BASE (S3C2410_CS4 + 0x300)

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
	&smdk2440_device_eth,
};
