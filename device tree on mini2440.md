# device tree on mini2440



## 1.version declare

u-boot : u-boot-2014.10

kernel : linux-3.10.79

gcc : arm-linux-4.4.3



## 2.download

支持设备树的加载命令格式如下：

bootm + uImage地址 + ramdisk地址 + 设备树镜像地址

## partition

```bash
 #: name		size		offset		mask_flags
 0: bootloader          0x00040000	0x00000000	0            # 256K (bootbloader)
 0: device_tree         0x00020000	0x00040000	0            # 128K (device tree)
 1: params              0x00020000	0x00060000	0            # 128K (params)
 2: kernel              0x00500000	0x00080000	0            # 5M
 3: rootfs              0x0fda0000	0x00580000	0            # ~
```



### u-boot

```bash
tftp 30000000 u-boot.bin
nand erase bootloader
nand write.jffs2 30000000 bootloader
```



### kernel

```bash
tftp 30000000 uImage
nand erase 0x00080000 0x500000 
nand write.jffs2 30000000 0x80000 0x500000
```

### dts

```bash
tftp 32000000 smdk2440.dtb
bootm 0x30007FC0 - 0x32000000 # （- 表示不使用ramdisk加载，如果使用ramdisk则提供其加载地址）
```



# u-boot

### 编译

```bash
make smdk2440_config ; make 
```

1.添加命令支持fdt：

vim include/configs/smdk2440.h

```c
#define CONFIG_OF_LIBFDT        1 /* add by Flinn for device tree */
```

### 修改分区

当前分区

```bash
 #: name		size		offset		mask_flags
 0: bootloader          0x00040000	0x00000000	0            # 256K
 1: params              0x00020000	0x00040000	0            # 128K
 2: kernel              0x00200000	0x00060000	0            # 2M
 3: rootfs              0x0fda0000	0x00260000	0            # ~

```

修改后的分区

```bash
 #: name		size		offset		mask_flags
 0: bootloader          0x00040000	0x00000000	0            # 256K (bootbloader)
 0: device_tree         0x00020000	0x00040000	0            # 128K (device tree)
 1: params              0x00020000	0x00060000	0            # 128K (params)
 2: kernel              0x00500000	0x00080000	0            # 5M
 3: rootfs              0x0fda0000	0x00580000	0            # ~
```

修改如下：

vim include/configs/smdk2440.h

```c
#define MTDPARTS_DEFAULT "mtdparts=smdk2440-0:256k(bootloader),"        \
                                        "128k(device_tree)"     \
                                        "128k(params),"         \
                                        "5m(kernel),"           \
                                        "-(rootfs)"    

-#define CONFIG_ENV_OFFSET 0x00040000
+#define CONFIG_ENV_OFFSET 0x00060000
```

### 修改环境参数

```c
#define CONFIG_BOOTCOMMAND	"nand read.jffs2 0x30007FC0 0x80000 0x500000;  bootm 0x30007FC0 "
```



# kernel

### 编译

```bash
make s3c2410_defconfig ; make uImage
```



### 修改分区

必须修改分区匹配u-boot

vim arch/arm/mach-s3c24xx/common-smdk.c

```c
static struct mtd_partition smdk_default_nand_part[] = {
        [0] = {
                .name   = "bootloader",
                .size   = SZ_256K,
                .offset = 0,
        },
        [1] = {
                .name   = "device_tree",
                .offset = MTDPART_OFS_APPEND,
                .size   = SZ_128K,
        },
        [2] = {
                .name   = "params",
                .offset = MTDPART_OFS_APPEND,
                .size   = SZ_128K,
        },
        [3] = {
                .name   = "kernel",
                .offset = MTDPART_OFS_APPEND,
                .size   = 0x500000,          // 5M
        },
        [4] = {
                .name   = "root",
                .offset = MTDPART_OFS_APPEND,
                .size   = MTDPART_SIZ_FULL,
        }
			};
```

如果乱码：

```bash
set bootargs noinitrd root=/dev/mtdblock4 rw init=/linuxrc console=ttySAC0,115200
```



# 设备树

1.最简单的设备树

```c
// SPDX-License-Identifier: GPL-2.0
/*
 * SAMSUNG SMDK2440 board device tree source
 *
 * Copyright (c) 2018 weidongshan@qq.com
 * dtc -I dtb -O dts -o mini2440.dts mini2440.dtb
 */
 
#define S3C2410_GPA(_nr)	((0<<16) + (_nr))
#define S3C2410_GPB(_nr)	((1<<16) + (_nr))
#define S3C2410_GPC(_nr)	((2<<16) + (_nr))
#define S3C2410_GPD(_nr)	((3<<16) + (_nr))
#define S3C2410_GPE(_nr)	((4<<16) + (_nr))
#define S3C2410_GPF(_nr)	((5<<16) + (_nr))
#define S3C2410_GPG(_nr)	((6<<16) + (_nr))
#define S3C2410_GPH(_nr)	((7<<16) + (_nr))
#define S3C2410_GPJ(_nr)	((8<<16) + (_nr))
#define S3C2410_GPK(_nr)	((9<<16) + (_nr))
#define S3C2410_GPL(_nr)	((10<<16) + (_nr))
#define S3C2410_GPM(_nr)	((11<<16) + (_nr))

/dts-v1/;

/ {
	model = "SMDK24440";
	compatible = "samsung,smdk2410", "samsung,smdk2440";

	#address-cells = <1>;
	#size-cells = <1>;
		
	memory@30000000 {
		device_type = "memory";
		reg =  <0x30000000 0x4000000>;
	};
/*
	cpus {
		cpu {
			compatible = "arm,arm926ej-s";
		};
	};
*/	
	chosen {
		bootargs = "noinitrd root=/dev/mtdblock4 rw init=/linuxrc console=ttySAC0,115200";
	};

	
	led {
		compatible = "smdk2440_led";
		reg = <S3C2410_GPF(5) 1>;
	};
};

```

修改启动参数

```
#define CONFIG_BOOTCOMMAND	"nand read.jffs2 0x30007FC0 kernel; nand read.jffs2 32000000 device_tree; bootm 0x30007FC0 - 0x32000000"
```

