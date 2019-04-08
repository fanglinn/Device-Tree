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

#include <linux/usb/input.h>
#include <linux/hid.h>


static struct input_dev *uk_dev;       //定义一个输入子系统设备结构体
static dma_addr_t usb_buf_phys;        //物理地址
static char *usb_buf;                  //从USB主控制器接收到的数据存放的导致
static int len;                        //从USB主控制器接收到的数据的长度
static struct urb *uk_urb;             //定义一个USB请求块



/* 一包数据接收完成后处理函数*/
 static void usb_mouse_as_key_irq(struct urb *urb)
 {
    /* 
      * USB鼠标数据含义
     *  data[0]: bit0-左键, 1-按下, 0-松开
     *          bit1-右键, 1-按下, 0-松开
     *          bit2-中键, 1-按下, 0-松开 
     */
     
    static unsigned char pre_val;//前一个按键的按键值，每当按键值变化才上传

    if((pre_val & (1<<0)) != (usb_buf[1] & (1<<0)))//左键发生变化
    {
         input_event(uk_dev,EV_KEY, KEY_L, (usb_buf[1]?1:0));
         input_sync(uk_dev);                                                    //上传同步事件
    }

    if((pre_val & (1<<1)) != (usb_buf[1] & (1<<1)))//右键发生变化
    {
         input_event(uk_dev,EV_KEY, KEY_S, (usb_buf[1]?1:0));
         input_sync(uk_dev);                                                    //上传同步事件
    }

    if((pre_val & (1<<2)) != (usb_buf[1] & (1<<2)))//中键发生变化
    {
         input_event(uk_dev,EV_KEY, KEY_ENTER, (usb_buf[1]?1:0));
         input_sync(uk_dev);                                                    //上传同步事件
    }
    pre_val = usb_buf[1];
    
     /* 重新提交urb */
    usb_submit_urb(uk_urb, GFP_KERNEL);//提交URB，将URB的控制还给USB核心处理程序
 }
 
static int usb_mouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
    struct usb_device *dev  = interface_to_usbdev(intf);//根据usb接口，取得usb设备
    struct usb_host_interface *interface;                        //定义一个USB主机控制器接口描述符
    struct usb_endpoint_descriptor *endpoint;                 //定义一个端点描述符
    int pipe;
	int ret = 0;
    
    interface = intf->cur_altsetting;                               //获得usb控制器的接口描述符

    endpoint = &interface->endpoint[0].desc;//取得usb 控制器的第一个端点描述符
    
    
    printk("found usbmouse!\n");

    printk("bcdUSB = %x\n",dev->descriptor.bcdUSB);  //从USB设备描述符中获取USB版本
    printk("vidUSB = %x\n",dev->descriptor.idVendor); //从USB设备描述符中获取厂商ID
    printk("pidUSB = %x\n",dev->descriptor.idProduct);//从USB设备描述符中获取产品ID

    printk("bdcUSB = %x\n",intf->cur_altsetting->desc.bInterfaceClass);//从USB设备获取设备类
    printk("bdsUSB = %x\n",intf->cur_altsetting->desc.bInterfaceSubClass);//从USB设备获取设备从类
    printk("bdpUSB = %x\n",intf->cur_altsetting->desc.bInterfaceProtocol);//从USB设备获取设备协议

    /* a、分配一个 input_dev结构体*/
    uk_dev = input_allocate_device();//分配一个input_dev结构体

    /* b、设置 */
        /* b.1 能产生哪类事件 */
    set_bit(EV_KEY, uk_dev->evbit);//产生按键事件
    set_bit(EV_REP, uk_dev->evbit);//产生重复事件
        /* b.2 能产生哪些事件 */
    set_bit(KEY_L, uk_dev->keybit);//产生按键事件的L事件
    set_bit(KEY_S, uk_dev->keybit);//产生按键事件的S事件
    set_bit(KEY_ENTER, uk_dev->keybit);//产生按键事件的ENTER时间
    /* c、注册 */
    input_register_device(uk_dev);//注册一个输入设备
    
    /* d、硬件相关的设置 */
    /* 数据传输三要素: 源、目的、长度*/
    /* 源：USB设备某个端点 */
    pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);//设置端点为中断IN端点
    
    /* 长度 */
    len = endpoint->wMaxPacketSize;//长度为最大包长度
    
    /* 目的 */
    usb_buf = usb_alloc_coherent(dev, len, GFP_ATOMIC, &usb_buf_phys);//分配一个地址作为USB接收到的数据
    
    /*　使用三要素*/
    uk_urb= usb_alloc_urb(0, GFP_KERNEL);              //分配一个USB请求块
    
    /* 使用三要素，设置urb */
    usb_fill_int_urb(uk_urb, dev, pipe, usb_buf,
                len,usb_mouse_as_key_irq, NULL, endpoint->bInterval);//初始化即将被发送到USB设备的中断端点的URB
    
    uk_urb->transfer_dma = usb_buf_phys;            //usb控制器完成数据接收后将数据存放的物理地址
    uk_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP; //当URB包含一个即将传输的DMA缓冲区时应该设置URB_NO_TRANSFER_DMA_MAP
    
    /* 使用URB */
    ret = usb_submit_urb(uk_urb, GFP_KERNEL);//提交urb
    if(ret)
        return -1;
    return 0;
}


static void usb_mouse_as_key_disconnect(struct usb_interface *intf)
{
    struct usb_device *dev = interface_to_usbdev(intf);
    
    input_free_device(uk_dev);//释放一个input_dev结构体
    input_unregister_device(uk_dev);//反注册一个输入设备
    //usb_buffer_free(dev, len, usb_buf, usb_buf_phys);//释放分配的usb缓存数据

	usb_free_coherent(dev, len, usb_buf, usb_buf_phys);
    usb_unlink_urb(uk_urb);//不使用urb控制块
    
    printk("disconnetc usbmouse\n");
}


static struct usb_device_id usb_mouse_as_key_id_table [] = {
    { USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
        USB_INTERFACE_PROTOCOL_MOUSE) },
    { }    /* Terminating entry *///终止入口项*/
};


static struct usb_driver usb_mouse_as_key_driver = {
    .name        = "usbmouse_askey",
    .probe        = usb_mouse_as_key_probe,
    .disconnect    = usb_mouse_as_key_disconnect,
    .id_table    = usb_mouse_as_key_id_table,
};

static int __init mini2440_usbmouse_init(void)
{
	int retval = usb_register(&usb_mouse_as_key_driver);//注册一个usb驱动
    return retval;
}


static void __exit mini2440_usbmouse_exit(void)
{
	usb_deregister(&usb_mouse_as_key_driver);
}

module_init(mini2440_usbmouse_init);
module_exit(mini2440_usbmouse_exit);
MODULE_LICENSE("GPL");

