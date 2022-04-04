#include <linux/module.h>
#include <linux/kernel.h>

static int __init fd_init(void) /* Constructor */
{
	printk(KERN_INFO "fd registered");
	return 0;
}

static void __exit fd_exit(void) /* Destructor */
{
	printk(KERN_INFO "fd unregistered");
}

module_init(fd_init);
module_exit(fd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anil Kumar Pugalia <anil@sysplay.in>");
MODULE_DESCRIPTION("My First Driver");
