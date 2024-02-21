// SPDX-License-Identifier: GPL-3.0-only
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Louis Solofrizzo <louis@ne02ptzero.me>");
MODULE_DESCRIPTION("Creates a file to write to and read in reverse from.");

static ssize_t	myfd_read(struct file *fp, char __user *user, size_t size,
			  loff_t *offs);

static ssize_t	myfd_write(struct file *fp, const char __user *user,
			   size_t size, loff_t *offs);

static const struct file_operations fops = {
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
static char	tmp[PAGE_SIZE];

static int __init myfd_init(void)
{
	unsigned int	ret = misc_register(&myfd_device);

	if (ret < 0) {
		pr_err("Registrering module %s failed: %d\n", myfd_device.name, ret);
		return ret;
	}
	buffer[0] = 0;
	pr_info("Module %s loaded\n", myfd_device.name);
	return 0;
}

static void __exit myfd_cleanup(void)
{
	misc_deregister(&myfd_device);
	pr_info("Module %s unloaded\n", myfd_device.name);
}

ssize_t	myfd_read(struct file *fp, char __user *user, size_t size,
		  loff_t *offs)
{
	ssize_t	i;
	ssize_t	j;

	j = strlen(buffer) - 1;
	i = 0;
	while (j >= 0) {
		tmp[i] = buffer[j];
		i++;
		j--;
	}
	tmp[i] = 0;
	return simple_read_from_buffer(user, size, offs, tmp, i);
}

static ssize_t myfd_write(struct file *fp, const char __user *user, size_t size,
			  loff_t *offs)
{
	ssize_t	res;

	res = simple_write_to_buffer(buffer, PAGE_SIZE - 1, offs, user, size);
	if (res >= 0)
		buffer[res] = '\0';
	return res;
}

module_init(myfd_init);
module_exit(myfd_cleanup);
