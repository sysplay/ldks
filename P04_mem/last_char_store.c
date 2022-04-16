#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/uaccess.h>

#define FIRST_MINOR 0
#define MINOR_CNT 1
//#define NCHAR 1

static dev_t dev;
static struct cdev c_dev;
static char store = '0';
#ifdef NCHAR
static int n = 1;
module_param(n, int, 0);

MODULE_PARM_DESC(n, "Max data read count");
#endif

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
	if (*off == 0)
	{
		if (copy_to_user(buf, &store, 1))
		{
			return -EFAULT;
		}
		(*off)++;
		return 1;
	}
	else
	{
		return 0;
	}
}
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	if (copy_from_user(&store, &buf[len - 1], 1))
	{
		return -EFAULT;
	}
	return len;
}

static struct file_operations driver_fops =
{
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.write = my_write
};

static int __init lcs_init(void)
{
	int ret;

	if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "lcs")) < 0)
	{
		return ret;
	}

	cdev_init(&c_dev, &driver_fops);
	if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
	{
		return ret;
	}

	return 0;
}

static void __exit lcs_exit(void)
{
	cdev_del(&c_dev);
	unregister_chrdev_region(dev, MINOR_CNT);
}

module_init(lcs_init);
module_exit(lcs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anil Kumar Pugalia <anil@sysplay.in>");
MODULE_DESCRIPTION("Last Character Store Char Driver");
