#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include <linux/vmstat.h>



struct memory_snapshot {
    unsigned long total_pages;
    unsigned long free_pages;
    unsigned long active_pages;
    unsigned long cache_pages;
    unsigned long swap_pages;
};


SYSCALL_DEFINE1(matus_capture_memory_snapshot, struct memory_snapshot __user*, user_snapshot) {

	if (!user_snapshot) {
		printk(KERN_INFO "Pased *user_snapshot is NULL");
		return -EINVAL;
	}

	struct memory_snapshot kernel_snapshot;
    struct sysinfo info;
    si_meminfo(&info);

    kernel_snapshot.total_pages  = 	info.totalram;
    kernel_snapshot.free_pages   = 	info.freeram;
	kernel_snapshot.active_pages = 	global_node_page_state(NR_ACTIVE_ANON) + global_node_page_state(NR_ACTIVE_FILE);
	kernel_snapshot.cache_pages  = 	global_node_page_state(NR_FILE_PAGES);
	kernel_snapshot.swap_pages   = 	global_node_page_state(NR_SWAPCACHE);


    if (copy_to_user(user_snapshot, &kernel_snapshot, sizeof(kernel_snapshot))) {
        return -EFAULT;
    }

    return 0;

}

