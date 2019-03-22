# Device-Tree
Device-Tree for MINI2440

export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games://home/flinn/tools/4.4.3/bin

set bootargs noinitrd init=/linuxrc console=ttySAC0,115200 root=/dev/nfs nfsroot=192.168.10.221:/home/flinn/Device-Tree/rootfs/rootfs  ip=192.168.10.123:192.168.10.221:192.168.10.1:255.255.255.0::eth0:off 


tftp 32000000 s3c2440-mini2440.dtb;tftp 30007FC0 uImage;bootm 0x30007FC0 - 0x32000000
