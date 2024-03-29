#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rkowalsk");
MODULE_DESCRIPTION("Using debugfs for the first time ever in my entire life.");

#define	MODULE_NAME "debog"

struct dentry		*parent_entry;
struct dentry		*id_entry;
struct dentry		*jiffies_entry;
struct dentry		*foo_entry;
static const char	login[] = "rkowalsk\n";
static size_t		login_size = strlen(login);
static char		foo_buffer[PAGE_SIZE];
static			DEFINE_MUTEX(foo_mutex);

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
	if (*offset == output_len) {
		*offset = 0;
		return (0);
	}
	while (count_read < len && i < output_len) {
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

static ssize_t	foo_read(struct file *filp, char __user *buffer, size_t len,
								loff_t *offset)
{
	size_t	i = *offset;
	size_t	count_read = 0;
	size_t	foo_size;
	mutex_lock(&foo_mutex);
	foo_size = strlen(foo_buffer);
	if (*offset >= foo_size) {
		*offset = 0;
		mutex_unlock(&foo_mutex);
		return (0);
	}
	while (count_read < len && i < foo_size) {
		if (put_user(foo_buffer[i++], buffer++)) {
			mutex_unlock(&foo_mutex);
			return (-1);
		}
		count_read++;
	}
	mutex_unlock(&foo_mutex);
	*offset = i;
	return (count_read);
}

static ssize_t	foo_write(struct file *filp, const char __user *buffer,
						size_t len, loff_t *offset)
{
	size_t		write_size;
	unsigned long	copy_ret;
	if (*offset >= PAGE_SIZE - 1)
		return (-EFBIG);
	if (PAGE_SIZE - *offset - 1 <= len)
		write_size = PAGE_SIZE - *offset - 1;
	else
		write_size = len;
	mutex_lock(&foo_mutex);
	copy_ret = copy_from_user(foo_buffer + *offset, buffer, write_size);
	mutex_unlock(&foo_mutex);
	if (copy_ret)
		return (-EFAULT);
	*offset += write_size;
	foo_buffer[*offset] = 0;
	return (write_size);
}

static struct file_operations	foo_fops = {
	.read = foo_read,
	.write = foo_write
};

static int __init parent_init(void)
{
	parent_entry = debugfs_create_dir("fortytwo", NULL);
	if (IS_ERR(parent_entry)) {
		long	err_code = PTR_ERR(parent_entry);
		pr_err("Registrering module %s failed: %ld\n", MODULE_NAME,
							err_code);
		return (-err_code);
	}
	return (0);
}

static int __init id_init(void)
{
	id_entry = debugfs_create_file("id", 0666, parent_entry, NULL,
								&id_fops);
	if (IS_ERR(id_entry)) {
		long	err_code = PTR_ERR(id_entry);
		pr_err("Registrering module %s failed: %ld\n", MODULE_NAME,
							err_code);
		return (-err_code);
	}
	return (0);
}

static int __init jiffies_init(void)
{
	jiffies_entry = debugfs_create_file("jiffies", 0444, parent_entry, NULL,
								&jiffies_fops);
	if (IS_ERR(jiffies_entry)) {
		long	err_code = PTR_ERR(jiffies_entry);
		pr_err("Registrering module %s failed: %ld\n", MODULE_NAME,
							err_code);
		return (-err_code);
	}
	return (0);
}

static int __init foo_init(void)
{
	foo_entry = debugfs_create_file("foo", 0644, parent_entry, NULL,
								&foo_fops);
	if (IS_ERR(foo_entry)) {
		long	err_code = PTR_ERR(foo_entry);
		pr_err("Registrering module %s failed: %ld\n", MODULE_NAME,
							err_code);
		return (-err_code);
	}
	foo_buffer[0] = '\0';
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
	err_code = foo_init();
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
