#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/dcache.h>
#include <linux/nsproxy.h>
#include <linux/fs_struct.h>
#include <../fs/mount.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rkowalsk");
MODULE_DESCRIPTION("/proc/mounts but less good.");

#define	MODULE_NAME "mymounts"

static struct proc_dir_entry	*entry;

static char	*get_mount_points(void)
{
	struct mount		*curr;
	struct list_head	*head;
	char			*output;
	char			*tmp;
	size_t			size;
	head = &current->nsproxy->mnt_ns->list;
	output = kmalloc(1, GFP_KERNEL);
	if (!output) {
		pr_err("Kmalloc failed in module %s.\n", MODULE_NAME);
		return (NULL);
	}
	output[0] = '\0';
//	down_read(&namespace_sem);
	list_for_each_entry(curr, head, mnt_list) {
		size = snprintf(NULL, 0, "%-10s\n",
					curr->mnt_mountpoint->d_name.name);
		size += strlen(output) + 1;
		tmp = kmalloc(sizeof(char) * size, GFP_KERNEL);
		if (!tmp) {
			pr_err("Kmalloc failed in module %s.\n",MODULE_NAME);
			kfree(output);
			return (NULL);
		}
		snprintf(tmp, size, "%s%-10s\n", output,
					curr->mnt_mountpoint->d_name.name);
		kfree(output);
		output = tmp;
	}
//	up_read(&namespace_sem);
	return (output);
}

static ssize_t	mymounts_read(struct file *filp, char __user *buffer,
				size_t buffer_len, loff_t *offset)
{
	char	*mount_list;
	size_t	read_size;
	mount_list = get_mount_points();
	if (!mount_list) {
		return (-EFAULT);
	}
	read_size = strlen(mount_list) - *offset;
	if (read_size <= 0) {
		*offset = 0;
		kfree(mount_list);
		return (0);
	}
	if (copy_to_user(buffer, mount_list + *offset, read_size)) {
		kfree(mount_list);
		return (-EFAULT);
	}
	*offset += read_size;
	kfree(mount_list);
	return (read_size);
}

static const struct proc_ops	pops = {
	.proc_read = mymounts_read
};

static int __init mymounts_init(void)
{
	entry = proc_create(MODULE_NAME, 0444, NULL, &pops);
	if (entry == NULL) {
		pr_alert("Error: Couldn't initialize /proc/%s\n", MODULE_NAME);
		return (-ENOMEM);
	}
	pr_info("Module /proc/%s loaded\n", MODULE_NAME);
	return (0);
}

static void __exit mymounts_cleanup(void)
{
	proc_remove(entry);
	pr_info("Module /proc/%s unloaded\n", MODULE_NAME);
}

module_init(mymounts_init);
module_exit(mymounts_cleanup);
