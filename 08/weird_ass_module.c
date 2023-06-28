#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Louis Solofrizzo <louis@ne02ptzero.me>");
MODULE_DESCRIPTION("Creates a file to write and read from.");

static ssize_t	myfd_read(struct file *fp, char __user *user, size_t size,
					loff_t *offs);

static ssize_t	myfd_write(struct file *fp, const char __user *user,
					size_t size, loff_t *offs);

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = &myfd_read,
	.write = &myfd_write
};

static struct miscdevice myfd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "reverse",
	.fops = &fops
};

static char	buffer[PAGE_SIZE];

static int __init myfd_init (void)
{
	return (misc_register(&myfd_device));
}

static void __exit myfd_cleanup (void)
{
}

ssize_t	myfd_read(struct file *fp, char __user *user, size_t size,
					loff_t *offs)
{
	size_t	buff_size = strlen(buffer);
	return simple_read_from_buffer(user, size, offs, buffer, buff_size);
}

static ssize_t myfd_write(struct file *fp, const char __user *user, size_t size,
					loff_t *offs)
{
	ssize_t	res;
	res = simple_write_to_buffer(buffer, PAGE_SIZE, offs, user, size) + 1;
	if (res > 0)
		buffer[res + 1] = '\0';
	return res;
}

module_init(myfd_init);
module_exit(myfd_cleanup);
