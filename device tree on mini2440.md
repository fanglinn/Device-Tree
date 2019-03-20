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

### 注意！！！

busybox使用arm-linux-gcc-4.4.3编译， 使用arm-linux-gcc-4.3.2的库文件 ！！！！！！！！！！！！！！！

### yaffs

```bash
tftp 30000000 rootfs.yaffs2
nand erase.part rootfs
nand write.yaffs 30000000 rootfs $filesize
```

### jffs2

```bash
tftp 30000000 rootfs.jffs2
nand erase.part rootfs
nand write.jffs2 30000000 rootfs $filesize
set bootargs noinitrd root=/dev/mtdblock4 rw init=/linuxrc console=ttySAC0,115200 rootfstype=jffs2
```



### 新建目录

​	mkdir ~/mini2440/rootfs/fs_new
​		

	tar xvf busybox-1.20.1.tar.bz2
	#make menuconfig   在setting中，制定交叉编译器
 	Busybox Settings  --->  
 	  Build Options  ---> 
 	  	 (arm-linux-) Cross Compiler prefix 
 	  	 

	/home/flinn/tools/4.4.3/bin/arm-linux-
	make
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

#cd /home/flinn/tools/4.3.2/
#find /home/flinn/tools/4.3.2/ -name lib
显示以下库：

```
./arm-none-linux-gnueabi/libc/usr/lib
./arm-none-linux-gnueabi/libc/armv4t/usr/lib
./arm-none-linux-gnueabi/libc/armv4t/lib
./arm-none-linux-gnueabi/libc/thumb2/usr/lib
./arm-none-linux-gnueabi/libc/thumb2/lib
./arm-none-linux-gnueabi/libc/lib
./arm-none-linux-gnueabi/lib
./lib
```




	 我们需要：
	./arm-none-linux-gnueabi/libc/armv4t/usr/lib
	./arm-none-linux-gnueabi/libc/armv4t/lib
#cd /work/tmp/first_fs/

```
 #mkdir lib
 #mkdir usr/lib -p
 #sudo cp /home/flinn/tools/4.3.2/arm-none-linux-gnueabi/libc/armv4t/lib/*so* ./lib -d
 #sudo cp /home/flinn/tools/4.3.2/arm-none-linux-gnueabi/libc/armv4t/usr/lib/*so* ./usr/lib -d
```

 最小根文件系统已经完成



### 制作文件系统

```bash
#sudo mkyaffs2image fs_mini_mdev_new fs_mini_mdev_new.yaffs2       
#sudo mkfs.jffs2 -n -s 2048 -e 128KiB -d fs_mini_mdev_new -o fs_mini_mdev_new.jffs2
```



### 烧录可用的文件系统

```bash
yaffs2
tftp 30000000 fs_mini_mdev_new.yaffs2
nand erase.part rootfs
nand write.yaffs 30000000 rootfs $filesize

jffs2
tftp 30000000 fs_mini_mdev_new.jffs2
nand erase.part rootfs
nand write.jffs2 30000000 rootfs $filesize
set bootargs noinitrd root=/dev/mtdblock4 rw init=/linuxrc console=ttySAC0,115200 rootfstype=jffs2
```



### 优化

当前不支持PROC

支持PROC

#vim  etc/inittab
		

```
console::askfirst:-/bin/sh
::sysinit:/etc/init.d/rcS
```


​	创建/etc/init.d/rcS 文件

		mkdir etc/init.d
		vim  etc/init.d/rcS
		mount -t proc none /proc
		chmod +x etc/init.d/rcS
使用mount -a

vim  /etc/init.d/rcS
		

```bash
#mount -t proc none /proc
mount -a
```

mount -a 依赖/etc/fstab

vim /etc/fstab

```bash
+ # device    mount-point      type   option    dump   fsck  order
+ proc         /proc            proc     defaults     0      0
```

支持mdev

cd rootfs
mkdir sys
vim etc/fstab

```bash
sysfs           /sys           sysfs    defaults     0      0
tmpfs           /dev           tmpfs    defaults     0      0
```

vim etc/init.d/rcS

```bash
mount -a 
mkdir /dev/pts/
mount -t devpts devpts /dev/pts
echo /sbin/mdev > /proc/sys/kernel/hotplug
mdev -s
```



## 工具编译

切换gcc版本：

```
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games://home/flinn/tools/4.4.3/bin
```



## NFS支持

### ENV

vim /etc/exports

```bash
/home/flinn/Device-Tree/rootfs/fs_mini_mdev_new *(rw,sync,no_root_squash)
```

重启

```bash
sudo /etc/init.d/nfs-kernel-server restart
```

### nfs烧录

```bash
fs-yaffs2:
	nfs 30000000 192.168.10.119:/home/flinn/Device-Tree/rootfs/fs_mini_mdev_new.yaffs2
	nand erase rootfs
	nand write.yaffs 30000000 rootfs 85b540
```

### nfs启动

```sh
set bootargs noinitrd init=/linuxrc console=ttySAC0,115200 root=/dev/nfs nfsroot=192.168.10.221:/work/rootfs  ip=192.168.10.123:192.168.10.221:192.168.10.1:255.255.255.0::eth0:off 
```

