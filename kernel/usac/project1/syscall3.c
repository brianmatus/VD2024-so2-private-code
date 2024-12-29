#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>

//#include <linux/ioacct.h>


#include <linux/gfp.h>
#include <asm/unistd.h>


//#include <linux/time.h>
//#include <linux/ktime.h>

struct io_stats {
    unsigned long bytes_read;
    unsigned long bytes_written;
    unsigned long bytes_read_disk;
    unsigned long bytes_written_disk;
    unsigned long io_wait_time;  // In ms
};



SYSCALL_DEFINE2(matus_get_io_throttle, pid_t, pid, struct io_stats __user *, stats) {
    struct task_struct *task;
    struct io_stats kstats;

    task = find_task_by_vpid(pid);
    if (!task) {
        printk(KERN_INFO "PID con numero %d no encontrado, cancelando get_io_throttle\n", pid);
        return -ESRCH;
    }


    task_lock(task);

    kstats.bytes_read = task->ioac.rchar;
    kstats.bytes_written = task->ioac.wchar;

    kstats.bytes_read_disk = task->ioac.read_bytes;
    kstats.bytes_written_disk = task->ioac.write_bytes;

    //TODO io wait time

    task_unlock(task);


    if (copy_to_user(stats, &kstats, sizeof(struct io_stats))) {
        printk(KERN_ERR "Error al copiar a user-space kstats ----> stats\n");
        return -EFAULT;
    }

    return 0;
}
