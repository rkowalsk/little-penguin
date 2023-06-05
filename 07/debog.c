#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/debugfs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rkowalsk");
MODULE_DESCRIPTION("Using debugfs for the first time ever in my entire life.");

#define	MODULE_NAME "debog"

struct dentry		*parent_entry;
struct dentry		*id_entry;
struct dentry		*jiffies_entry;
static const char	login[] = "rkowalsk\n";
static size_t		login_size = strlen(login);

static ssize_t	id_write(struct file *filp, const char __user *buffer,
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

static ssize_t	id_read(struct file *filp, char __user *buffer, size_t len,
								loff_t *offset)
{
	size_t	i = *offset;
	size_t	count_read = 0;

	if (*offset == login_size)
	{
		*offset = 0;
		return (0);
	}
	while (count_read < len && i < login_size)
	{
		if (put_user(login[i++], buffer++))
			return (-1);
		count_read++;
	}
	*offset = i;
	return (count_read);
}

static struct file_operations	id_fops = {
	.read = id_read,
	.write = id_write
};

static ssize_t	jiffies_read(struct file *filp, char __user *buffer, size_t len,
								loff_t *offset)
{
	u64	timestamp = get_jiffies_64();
	char	output[20];
	size_t	output_len;
	size_t	i = *offset;
	size_t	count_read = 0;
	output_len = snprintf(output, 20, "%llu\n", timestamp);
	if (*offset == output_len)
	{
		*offset = 0;
		return (0);
	}
	while (count_read < len && i < output_len)
	{
		if (put_user(output[i++], buffer++))
			return (-1);
		count_read++;
	}
	*offset = i;
	return (count_read);
}

static struct file_operations	jiffies_fops = {
	.read = jiffies_read
};

static int __init parent_init(void)
{
	parent_entry = debugfs_create_dir("fortytwo", NULL);
	if (IS_ERR(parent_entry))
	{
		long	err_code = PTR_ERR(parent_entry);
		pr_err("Registrering module %s failed: %ld\n", MODULE_NAME,
							err_code);
		return (-err_code);
	}
	return (0);
}

static int __init id_init(void)
{
	id_entry = debugfs_create_file("id", 770, parent_entry, NULL,
								&id_fops);
	if (IS_ERR(id_entry))
	{
		long	err_code = PTR_ERR(id_entry);
		pr_err("Registrering module %s failed: %ld\n", MODULE_NAME,
							err_code);
		return (-err_code);
	}
	return (0);
}

static int __init jiffies_init(void)
{
	jiffies_entry = debugfs_create_file("jiffies", 700, parent_entry, NULL,
								&jiffies_fops);
	if (IS_ERR(jiffies_entry))
	{
		long	err_code = PTR_ERR(jiffies_entry);
		pr_err("Registrering module %s failed: %ld\n", MODULE_NAME,
							err_code);
		return (-err_code);
	}
	return (0);
}

static int __init debog_init(void)
{
	int	err_code;

	err_code = parent_init();
	if (err_code)
		return (err_code);
	err_code = id_init();
	if (err_code)
		return (err_code);
	err_code = jiffies_init();
	if (err_code)
		return (err_code);
	pr_info("Module %s loaded\n", MODULE_NAME);
	return (0);
}

static void __exit debog_cleanup(void)
{
	debugfs_remove_recursive(parent_entry);
	pr_info("Module %s unloaded\n", MODULE_NAME);
}

module_init(debog_init);
module_exit(debog_cleanup);
