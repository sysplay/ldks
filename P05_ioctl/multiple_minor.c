#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/uaccess.h>

#define FIRST_MINOR 0
#define MINOR_CNT 3

static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static char store = '0';

static int my_open(struct inode *i, struct file *f)
{
	f->private_data = (void *)((long)(iminor(i)));
	return 0;
}

static int my_close(struct inode *i, struct file *f)
{
	return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	int i;

	switch ((long)(f->private_data))
	{
		case 0:
			i = 0;
			while (i < len)
			{
				if (copy_to_user(&buf[i], &store, 1) != 0)
				{
					return -EFAULT;
				}
				i++;
			}
			(*off) += len;
			return len;
			break;
		case 1:
			/* TODO 1: Implement the null driver's read here */
			break;
	}

	return -EINVAL;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	switch ((long)(f->private_data))
	{
		case 0:
			if (copy_from_user(&store, &buf[len - 1], 1) != 0)
			{
				return -EFAULT;
			}
			return len;
			break;
		case 1:
			/* TODO 2: Implement the null driver's write here */
			break;
	}

	return -EINVAL;
}

static struct file_operations driver_fops =
{
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.write = my_write
};

static int __init multiple_minor_init(void)
{
	int ret, i;
	struct device *dev_ret;

	if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "multiple_minor")) < 0)
	{
		return ret;
	}

	cdev_init(&c_dev, &driver_fops);
	if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
	{
		unregister_chrdev_region(dev, MINOR_CNT);
		return ret;
	}

	if (IS_ERR(cl = class_create(THIS_MODULE, "char")))
	{
		cdev_del(&c_dev);
		unregister_chrdev_region(dev, MINOR_CNT);
		return PTR_ERR(cl);
	}
	for (i = 0; i < MINOR_CNT; i++)
	{
		if (IS_ERR(dev_ret =
				device_create(cl, NULL, MKDEV(MAJOR(dev), FIRST_MINOR + i), NULL, "mm%d", FIRST_MINOR + i)))
		{
			for (--i; i >= 0; i--)
				device_destroy(cl, MKDEV(MAJOR(dev), FIRST_MINOR + i));
			class_destroy(cl);
			cdev_del(&c_dev);
			unregister_chrdev_region(dev, MINOR_CNT);
			return PTR_ERR(dev_ret);
		}
	}

	return 0;
}

static void __exit multiple_minor_exit(void)
{
	int i;

	for (i = 0; i < MINOR_CNT; i++)
	{
		device_destroy(cl, MKDEV(MAJOR(dev), FIRST_MINOR + i));
	}
	class_destroy(cl);
	cdev_del(&c_dev);
	unregister_chrdev_region(dev, MINOR_CNT);
}

module_init(multiple_minor_init);
module_exit(multiple_minor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anil Kumar Pugalia <anil@sysplay.in>");
MODULE_DESCRIPTION("Multiple Minor-ed Char Driver");
