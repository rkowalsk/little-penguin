#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Romain Kowalski");
MODULE_DESCRIPTION("A simple Hello World module");

static int __init hello_world_init(void)
{
	printk(KERN_INFO"Hello World\n");
	return (0);
}

static void __exit hello_world_cleanup(void)
{
	printk(KERN_INFO"J'me tire\n");
}

module_init(hello_world_init);
module_exit(hello_world_cleanup);
