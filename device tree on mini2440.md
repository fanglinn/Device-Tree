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



NFS下载

```bash
nfs 30000000 192.168.10.140:/home/flinn/bin/uImage
nand erase 0x00080000 0x500000
nand write.jffs2 30000000 0x80000 0x500000
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

### 添加对设备树的支持

vim arch/arm/mach-s3c24xx/mach-smdk2440.c

```c
static void __init smdk2440_init_time(void)
{
    s3c2440_init_clocks(12000000);    
}

/* add by Flinn */
static const char *const smdk2440_dt_compat[] __initconst = {
        "samsung,smdk2440", 
        NULL
};

MACHINE_START(S3C2440, "SMDK2440")
	.dt_compat      = smdk2440_dt_compat,   /* add by Flinn */
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









# 根文件系统

### yaffs

```
tftp 30000000 rootfs.yaffs2
nand erase.part rootfs
nand write.yaffs 30000000 rootfs 
```

### jffs2

```
tftp 30000000 rootfs.jffs2
nand erase.part rootfs
nand write.jffs2 30000000 rootfs 
set bootargs noinitrd root=/dev/mtdblock4 rw init=/linuxrc console=ttySAC0,115200 rootfstype=jffs2
```



### 新建目录

​	mkdir ~/mini2440/rootfs/fs_new
​		

	tar xvf busybox-1.22.1.tar.bz2
	#make menuconfig   在setting中，制定交叉编译器
 	Busybox Settings  --->  
 	  Build Options  ---> 
 	  	 (arm-linux-) Cross Compiler prefix 
 	  	 
	/home/flinn/tools/4.4.3/bin/arm-none-linux-gnueabi-
	make CONFIG_PREFIX=/home/flinn/Device-Tree/rootfs/rootfs install
### 创建console

现在上面编译后busybox目录是work/tmp/first_fs
	在此目录下：ls /dev/console /dev/null -l
	crw------- 1 root root 5, 1 2015-01-05 20:57 /dev/console
	crw-rw-rw- 1 root root 1, 3 2015-01-05 20:30 /dev/null
	那么根据它来创建console null等设备
	

	#mkdir dev
	#cd dev
	#mknod console c 5 1 
	#mknod null c 1 3
	#ls -l
	显示：
	 crw-r--r-- 1 root root 5, 1 2015-05-06 20:39 console
	crw-r--r-- 1 root root 1, 3 2015-05-06 20:40 null
	表示创建成功


### 配置项

#mkdir etc
#vim  etc/inittab
	

	输入：
	console::askfirst:-/bin/sh


### 安装c库

#cd /home/flinn/tools/4.4.3/
#find /home/flinn/tools/4.4.3/ -name lib
	显示以下库：
		-/home/flinn/tools/4.4.3/arm-none-linux-gnueabi/debug-root/usr/lib
		/home/flinn/tools/4.4.3/arm-none-linux-gnueabi/lib
		/home/flinn/tools/4.4.3/arm-none-linux-gnueabi/sys-root/usr/lib
		/home/flinn/tools/4.4.3/arm-none-linux-gnueabi/sys-root/lib
		/home/flinn/tools/4.4.3/lib


	 我们需要：
	 	/home/flinn/tools/4.4.3/arm-none-linux-gnueabi/sys-root/usr/lib
		/home/flinn/tools/4.4.3/arm-none-linux-gnueabi/sys-root/lib
#cd /work/tmp/first_fs/
 #mkdir lib
 #mkdir usr/lib -p
 #sudo cp /home/flinn/tools/4.4.3/arm-none-linux-gnueabi/sys-root/usr/lib/*so* ./lib -d
 #sudo cp /home/flinn/tools/4.4.3/arm-none-linux-gnueabi/sys-root/usr/lib/*so* ./usr/lib -d

 最小根文件系统已经完成



### 制作文件系统

```bash
#sudo mkfs.jffs2 -n -s 2048 -e 128KiB -d rootfs -o rootfs.jffs2
#sudo mkyaffs2image_new rootfs rootfs.yaffs2        // mkyaffs2image_new!!!!!!!!!!!!!!!!!!!!!!!!!!!
```

