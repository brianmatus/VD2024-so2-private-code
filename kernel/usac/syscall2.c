#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>


#include <linux/time.h>
#include <linux/ktime.h>

#include "syscall2.h"

struct syscall_usage {
	unsigned long count;
	struct timespec64 time_last_used;
};


static struct syscall_usage *syscall_counters;


#define SYS_CALL_ARRAY_SIZE 4
static const int syscall_whitelist[] = {0, 1, 2, 56}; //read,write,open,fork


static int init_syscall_counters(void) {
	syscall_counters = kzalloc(sizeof(struct syscall_usage)*SYS_CALL_ARRAY_SIZE,GFP_KERNEL);
	printk(KERN_INFO "syscall_counters allocated: %p\n", syscall_counters);
	return syscall_counters ? 0 : -ENOMEM;
}


static int syscall_tracked_index(int syscall_id) {
    int i;
    for (i = 0; i < SYS_CALL_ARRAY_SIZE; i++) {
        if (syscall_whitelist[i] == syscall_id) {
            return i;
        }
    }
    return -1;
}

void track_syscall(int syscall_id) {

	struct timespec64 now;

	if (!syscall_counters) {
		return;
	}
	
	int syscall_index = syscall_tracked_index(syscall_id);
	
	if (syscall_index == -1) {
		return;
	}

	syscall_counters[syscall_index].count++;
	ktime_get_real_ts64(&now);
	syscall_counters[syscall_index].time_last_used = now;
	
	//TODO Comprobar si esto funciona???
	//ktime_get_real_ts64(&syscall_counters[syscall_id].time_last_used);
}


SYSCALL_DEFINE1(matus_track_syscall_usage, struct syscall_usage*, statistics) {

    if (!syscall_counters) {
        printk(KERN_INFO "syscall_counters is NULL, calling init_syscall_counters()");
        if (init_syscall_counters() != 0) {
            printk(KERN_ERR "call to init_syscall_counters() returned non-zero error code");
            return -ENOMEM;
        }
    }

    if (copy_to_user(statistics, syscall_counters, sizeof(struct syscall_usage) * SYS_CALL_ARRAY_SIZE)) {
        return -EFAULT;
    }

    return 0;
}


