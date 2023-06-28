#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/errno.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rkowalsk");
MODULE_DESCRIPTION("42 nerd module");

#define	MODULE_NAME "forty_two"
#define	DEVICE_NAME "fortytwo"

static const char	login[] = "rkowalsk\n";
static size_t		login_size = strlen(login);

static ssize_t	ft_write(struct file *filp, const char __user *buffer,
						size_t len, loff_t *offset)
{
	char	local_buff[10];

	if (len != login_size)
		return (-EINVAL);
	if (copy_from_user(local_buff, buffer, len))
		return (-EFAULT);
	if (memcmp(local_buff, login, len))
		return (-EINVAL);
	return (login_size);
}

static ssize_t	ft_read(struct file *filp, char __user *buffer, size_t len,
								loff_t *offset)
{
	size_t	i = *offset;
	size_t	count_read = 0;

	if (*offset == login_size) {
		*offset = 0;
		return (0);
	}
	while (count_read < len && i < login_size) {
		if (put_user(login[i++], buffer++))
			return (-1);
		count_read++;
	}
	*offset = i;
	return (count_read);
}

static struct file_operations	fops = {
	.read = ft_read,
	.write = ft_write
};

static struct miscdevice	misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &fops,
};

static int __init fortytwo_init(void)
{
	unsigned int	ret = misc_register(&misc);
	if (ret < 0) {
		pr_err("Registrering module %s failed: %d\n", MODULE_NAME, ret);	
		return (ret);
	}
	pr_info("Module %s loaded\n", MODULE_NAME);
	return (0);
}

static void __exit fortytwo_cleanup(void)
{
	misc_deregister(&misc);
	pr_info("Module %s unloaded\n", MODULE_NAME);
}

module_init(fortytwo_init);
module_exit(fortytwo_cleanup);
