#include <linux/kernel.h>
#include <linux/syscalls.h>

#include <linux/time.h>
#include <linux/ktime.h>

SYSCALL_DEFINE0(matus_get_realtime)
{
    struct timespec64 ts;
    ktime_get_real_ts64(&ts);
    return ts.tv_sec;

}
