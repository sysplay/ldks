#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>

//#define HACK_BYPASS // Uncomment, to hack the driver to load

#ifndef HACK_BYPASS
#include <linux/ioport.h>
#endif

#define VRAM_BASE 0x000A0000
#define VRAM_SIZE 0x00020000

static void __iomem *vram;
static dev_t first;
static struct cdev c_dev;
static struct class *cl;

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

	if (*off >= VRAM_SIZE)
	{
		return 0;
	}
	if (*off + len > VRAM_SIZE)
	{
		len = VRAM_SIZE - *off;
	}
	for (i = 0; i < len; i++)
	{
		byte = ioread8((u8 *)vram + *off + i);
		if (copy_to_user(buf + i, &byte, 1))
		{
			return -EFAULT;
		}
	}
	*off += len;

	return len;
}
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	int i;
	u8 byte;

	if (*off >= VRAM_SIZE)
	{
		return 0;
	}
	if (*off + len > VRAM_SIZE)
	{
		len = VRAM_SIZE - *off;
	}
	for (i = 0; i < len; i++)
	{
		if (copy_from_user(&byte, buf + i, 1))
		{
			return -EFAULT;
		}
		iowrite8(byte, (u8 *)vram + *off + i);
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
			if ((offset >= 0) && (offset <= VRAM_SIZE))
				ret = f->f_pos = offset;
			break;
		case SEEK_CUR:
			if ((f->f_pos + offset >= 0) && (f->f_pos + offset <= VRAM_SIZE))
				ret = f->f_pos += offset;
			break;
		case SEEK_END:
			if ((VRAM_SIZE + offset >= 0) && (VRAM_SIZE + offset <= VRAM_SIZE))
				ret = f->f_pos = VRAM_SIZE + offset;
			break;
	}
	return ret;
}

static struct file_operations vram_fops =
{
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.write = my_write,
	.llseek = my_lseek
};

static int __init vram_init(void) /* Constructor */
{
	int ret;
#ifndef HACK_BYPASS
	struct resource *r;
#endif
	struct device *dev_ret;

#ifndef HACK_BYPASS
	if ((r = request_mem_region((resource_size_t)VRAM_BASE, (resource_size_t)VRAM_SIZE, "vram")) == NULL)
	{
		return -EBUSY;
	}
#endif
	if ((vram = ioremap(VRAM_BASE, VRAM_SIZE)) == NULL)
	{
		printk(KERN_ERR "Mapping video RAM failed\n");
		ret = -ENOMEM;
		goto rmr;
	}

	if ((ret = alloc_chrdev_region(&first, 0, 1, "vram")) < 0)
	{
		iounmap(vram);
		goto rmr;
	}
	cdev_init(&c_dev, &vram_fops);
	if ((ret = cdev_add(&c_dev, first, 1)) < 0)
	{
		unregister_chrdev_region(first, 1);
		iounmap(vram);
		goto rmr;
	}

	if (IS_ERR(cl = class_create(THIS_MODULE, "video")))
	{
		cdev_del(&c_dev);
		unregister_chrdev_region(first, 1);
		iounmap(vram);
		ret = PTR_ERR(cl);
		goto rmr;
	}
	if (IS_ERR(dev_ret = device_create(cl, NULL, first, NULL, "vram")))
	{
		class_destroy(cl);
		cdev_del(&c_dev);
		unregister_chrdev_region(first, 1);
		iounmap(vram);
		ret = PTR_ERR(dev_ret);
		goto rmr;
	}

	return 0;

rmr:
#ifndef HACK_BYPASS
	release_mem_region((resource_size_t)VRAM_BASE, (resource_size_t)VRAM_SIZE);
#endif
	return ret;
}

static void __exit vram_exit(void) /* Destructor */
{
	device_destroy(cl, first);
	class_destroy(cl);
	cdev_del(&c_dev);
	unregister_chrdev_region(first, 1);
	iounmap(vram);
#ifndef HACK_BYPASS
	release_mem_region((resource_size_t)VRAM_BASE, (resource_size_t)VRAM_SIZE);
#endif
}

module_init(vram_init);
module_exit(vram_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anil Kumar Pugalia <anil@sysplay.in>");
MODULE_DESCRIPTION("Video RAM Driver");
