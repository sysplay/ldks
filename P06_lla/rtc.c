#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define HACK_BYPASS // Uncomment, to hack the driver to load

#ifndef HACK_BYPASS
#include <linux/ioport.h>
#endif

#include "rtc.h"

#define RTC_REG_BASE 0x0070
#define RTC_REG_SIZE 0x0010
#define RTC_INDEX_PORT 0x0070
#define RTC_DATA_PORT 0x0071
#define NMI_DISABLE 0x80

static dev_t first;
static struct cdev c_dev;
static struct class *cl;

static int bcd_to_dec(u8 bcd)
{
	return (10 * (bcd >> 4) + (bcd & 0xF));
}
static u8 dec_to_bcd(int dec)
{
	return (u8)(((dec / 10) << 4) | (dec % 10));
}

static int my_open(struct inode *i, struct file *f)
{
	return 0;
}
static int my_close(struct inode *i, struct file *f)
{
	return 0;
}
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	int i;
	u8 byte;

	if (*off >= RTC_REG_SIZE)
	{
		return 0;
	}
	if (*off + len > RTC_REG_SIZE)
	{
		len = RTC_REG_SIZE - *off;
	}
	for (i = 0; i < len; i++)
	{
		byte = inb(RTC_REG_BASE + *off + i);
		if (copy_to_user(buf + i, &byte, 1))
		{
			return -EFAULT;
		}
	}
	*off += len;

	return len;
}
loff_t my_lseek(struct file *f, loff_t offset, int whence)
{
	loff_t ret = -EINVAL;

	switch (whence)
	{
		case SEEK_SET:
			if ((offset >= 0) && (offset <= RTC_REG_SIZE))
				ret = f->f_pos = offset;
			break;
		case SEEK_CUR:
			if ((f->f_pos + offset >= 0) && (f->f_pos + offset <= RTC_REG_SIZE))
				ret = f->f_pos += offset;
			break;
		case SEEK_END:
			if ((RTC_REG_SIZE + offset >= 0) && (RTC_REG_SIZE + offset <= RTC_REG_SIZE))
				ret = f->f_pos = RTC_REG_SIZE + offset;
			break;
	}
	return ret;
}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int my_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
static long my_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif
{
	int rw; // 0: read, 1: write
	u8 reg, bcd;
	int dec;

	/*
	 * TODOs: Fill in the appropriate register numbers below as per the cases of switch below
	 * You may use the following references to get the register numbers:
	 * + https://bochs.sourceforge.io/techspec/PORTS.LST
	 * + https://wiki.osdev.org/CMOS#The_Real-Time_Clock
	 */
	switch (cmd)
	{
		case RTC_GET_SECOND:
			rw = 0;
			reg = 0;
			break;
		case RTC_SET_SECOND:
			rw = 1;
			reg = 0;
			if (arg >= 60) return -EINVAL;
			break;
		case RTC_GET_MINUTE:
			rw = 0;
			reg = 0;
			break;
		case RTC_SET_MINUTE:
			rw = 1;
			reg = 0;
			if (arg >= 60) return -EINVAL;
			break;
		case RTC_GET_HOUR:
			rw = 0;
			reg = 0;
			break;
		case RTC_SET_HOUR:
			rw = 1;
			reg = 0;
			if (arg >= 24) return -EINVAL;
			break;
		case RTC_GET_DAY:
			rw = 0;
			reg = 0;
			break;
		case RTC_SET_DAY:
			rw = 1;
			reg = 0;
			if ((arg == 0) || (arg > 7)) return -EINVAL;
			break;
		case RTC_GET_DATE:
			rw = 0;
			reg = 0;
			break;
		case RTC_SET_DATE:
			rw = 1;
			reg = 0;
			if ((arg == 0) || (arg > 31)) return -EINVAL;
			break;
		case RTC_GET_MONTH:
			rw = 0;
			reg = 0;
			break;
		case RTC_SET_MONTH:
			rw = 1;
			reg = 0;
			if ((arg == 0) || (arg > 12)) return -EINVAL;
			break;
		case RTC_GET_YEAR:
			rw = 0;
			reg = 0;
			break;
		case RTC_SET_YEAR:
			rw = 1;
			reg = 0;
			if (arg > 99) return -EINVAL;
			break;
		default:
			printk(KERN_ERR "Invalid ioctl cmd: 0x%08X\n", cmd);
			return -EINVAL;
	}
	outb(NMI_DISABLE | reg, RTC_INDEX_PORT);
	if (!rw)
	{
		dec = bcd_to_dec(inb(RTC_DATA_PORT));
		if (copy_to_user((int *)(arg), &dec, sizeof(int)))
		{
			return -EFAULT;
		}
	}
	else
	{
		bcd = dec_to_bcd(arg);
		outb(bcd, RTC_DATA_PORT);
	}

	return 0;
}

static struct file_operations rtc_fops =
{
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.llseek = my_lseek,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
	.ioctl = my_ioctl
#else
	.unlocked_ioctl = my_ioctl
#endif
};

static int __init rtc_init(void) /* Constructor */
{
	int ret;
#ifndef HACK_BYPASS
	struct resource *r;
#endif
	struct device *dev_ret;

#ifndef HACK_BYPASS
	if ((r = request_region((resource_size_t)RTC_REG_BASE, (resource_size_t)RTC_REG_SIZE, "clock")) == NULL)
	{
		return -EBUSY;
	}
#endif

	if ((ret = alloc_chrdev_region(&first, 0, 1, "clock")) < 0)
	{
		goto rr;
	}
	cdev_init(&c_dev, &rtc_fops);
	if ((ret = cdev_add(&c_dev, first, 1)) < 0)
	{
		unregister_chrdev_region(first, 1);
		goto rr;
	}

	if (IS_ERR(cl = class_create(THIS_MODULE, "clock")))
	{
		cdev_del(&c_dev);
		unregister_chrdev_region(first, 1);
		ret = PTR_ERR(cl);
		goto rr;
	}
	if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, "clock")))
	{
		class_destroy(cl);
		cdev_del(&c_dev);
		unregister_chrdev_region(first, 1);
		ret = PTR_ERR(dev_ret);
		goto rr;
	}

	return 0;

rr:
#ifndef HACK_BYPASS
	release_region((resource_size_t)RTC_REG_BASE, (resource_size_t)RTC_REG_SIZE);
#endif
	return ret;
}

static void __exit rtc_exit(void) /* Destructor */
{
	device_destroy(cl, first);
	class_destroy(cl);
	cdev_del(&c_dev);
	unregister_chrdev_region(first, 1);
#ifndef HACK_BYPASS
	release_region((resource_size_t)RTC_REG_BASE, (resource_size_t)RTC_REG_SIZE);
#endif
}

module_init(rtc_init);
module_exit(rtc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anil Kumar Pugalia <anil@sysplay.in>");
MODULE_DESCRIPTION("Real Time Clock Driver");
