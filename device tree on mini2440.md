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
#define CONFIG_BOOTARGS	"noinitrd root=/dev/mtdblock4 rw init=/linuxrc console=ttySAC0,115200 "
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
 * Copyright (c) 2018 Flinn682@foxmail.com
 * dtc -I dtb -O dts -o mini2440.dts mini2440.dtb
 */
 

/dts-v1/;

/ {
	model = "SMDK2440";
	compatible = "samsung, s3c2410","samsung,smdk2410", "samsung,smdk2440";

	#address-cells = <1>;
	#size-cells = <1>;
		
	memory@30000000 {
		device_type = "memory";
		reg =  <0x30000000 0x4000000>;
	};

	chosen {
		bootargs = "noinitrd root=/dev/mtdblock4 rw init=/linuxrc console=ttySAC0,115200";
	};
};

```

修改启动参数

```
#define CONFIG_BOOTCOMMAND	"nand read.jffs2 0x30007FC0 kernel; nand read.jffs2 32000000 device_tree; bootm 0x30007FC0 - 0x32000000"
```

配置内核支持设备树

```bash
make menuconfig
    Boot options  ---> 
    	[*] Flattened Device Tree support 
```

make uImage dtbs

烧录dtb文件到device_tree分区：

```bash
tftp 32000000 mini2440.dtb
nand erase 0x00040000 0x20000   # nand erase.part device_tree
nand write.jffs2 32000000 0x40000 0x20000  # nand write.jffs2 30000000 device_tree
```

启动：

```bash
nand read.jffs2 0x30007FC0 kernel;     # 读内核uImage到内存0x30007FC0
nand read.jffs2 32000000 device_tree;  # 读dtb到内存32000000
bootm 0x30007FC0 - 0x32000000          # 启动, 没有initrd时对应参数写为"-"
```

快速测试：

```bash
tftp 32000000 mini2440.dtb
tftp 30007FC0 uImage
bootm 0x30007FC0 - 0x32000000 
or
tftp 32000000 mini2440.dtb;tftp 30007FC0 uImage;bootm 0x30007FC0 - 0x32000000 
```

问题：

```
mini2440 # bootm 0x30007FC0 - 0x31000000

## Current stack ends at 0x33cffda0 *  kernel: cmdline image address = 0x30007fc0

## Booting kernel from Legacy Image at 30007fc0 ...

   Image Name:   Linux-3.10.79
   Created:      2019-03-01   2:42:44 UTC
   Image Type:   ARM Linux Kernel Image (uncompressed)
   Data Size:    2607344 Bytes = 2.5 MiB
   Load Address: 30108000
   Entry Point:  30108000
   Verifying Checksum ... OK
   kernel data at 0x30008000, len = 0x0027c8f0 (2607344)

Skipping init Ramdisk
No init Ramdisk
   ramdisk start = 0x00000000, ramdisk end = 0x00000000
- fdt: cmdline image address = 0x31000000
Checking for 'FDT'/'FDT Image' at 31000000
ERROR: Did not find a cmdline Flattened Device Tree
Could not find a valid device tree
Command failed, result=1mini2440 #
```

