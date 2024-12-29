#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#include "usac_f3_main.h"


struct memory_limitation {
    pid_t pid;
    size_t memory_limit;
};

struct memory_limit_entry {
    struct memory_limitation data;
    struct list_head list;
};

static LIST_HEAD(memory_list);
static DEFINE_MUTEX(memory_list_mutex);


//TODO function ---> bool is_request_accepted(max, total, requested)

SYSCALL_DEFINE2(matus_add_memory_limit, pid_t, process_pid, size_t, memory_limit)
{
	//TODO chequeos de argumentos y errores
	//TODO chequeo de sudoer

	struct memory_limit_entry *entry;
	entry = kmalloc(sizeof(struct memory_limit_entry), GFP_KERNEL);
	if (!entry) {
		return -ENOMEM;
	}

	entry->data.pid = process_pid;
	entry->data.memory_limit = memory_limit;

	mutex_lock(&memory_list_mutex);
	list_add(&entry->list, &memory_list);
	mutex_unlock(&memory_list_mutex);

	print_memory_limitation_list();
	return 0;
}


SYSCALL_DEFINE3(matus_get_memory_limits, struct memory_limitation*, u_processes_buffer, size_t, max_entries, int*, processes_returned)
{
	//TODO manejo de errores, argumentos, sudoers, etc.
	struct memory_limit_entry * entry;
	int count = 0;

	//Memoria para el buffer del lado del kernel
	struct memory_limitation* k_processes_buffer = kmalloc_array(max_entries, sizeof(struct memory_limitation), GFP_KERNEL);
	if (!k_processes_buffer) {
		return -ENOMEM;
	}

	mutex_lock(&memory_list_mutex);
	list_for_each_entry(entry, &memory_list, list) {
		if (count >= max_entries) {
			break;
		}

		k_processes_buffer[count].pid = entry->data.pid;
		k_processes_buffer[count].memory_limit = entry->data.memory_limit;
		count++;
	}
	mutex_unlock(&memory_list_mutex);

	if (copy_to_user(u_processes_buffer, k_processes_buffer, count*sizeof(struct memory_limitation))) {
		kfree(k_processes_buffer);
		return -EFAULT;
	}

	if (put_user(count, processes_returned)) {
		kfree(k_processes_buffer);
		return -EFAULT;
	}

	kfree(k_processes_buffer);
	print_memory_limitation_list();
	return 0;
}


SYSCALL_DEFINE2(matus_update_memory_limit, pid_t, process_pid, size_t, memory_limit)
{
	//TODO manejo de argumentos, errores, sudoers, etc


	struct memory_limit_entry * entry;

	mutex_lock(&memory_list_mutex);
	list_for_each_entry(entry, &memory_list, list) {
		if (entry->data.pid == process_pid) {
			entry->data.memory_limit = memory_limit;
			mutex_unlock(&memory_list_mutex);
			print_memory_limitation_list();
			return 0;
		}
	}
	mutex_unlock(&memory_list_mutex);
	return -ESRCH;
}


SYSCALL_DEFINE1(matus_remove_memory_limit, pid_t, process_pid)
{
	//TODO manejo de errores y argumentos
	//TODO sudoer

	struct memory_limit_entry * entry, * tmp;

	mutex_lock(&memory_list_mutex);

	//FIXME change to only list_for_each_entry
	list_for_each_entry_safe(entry, tmp, &memory_list, list) {
		if (entry->data.pid == process_pid) {
			list_del(&entry->list);
			kfree(entry);
			mutex_unlock(&memory_list_mutex);
			print_memory_limitation_list();
			return 0;
		}
	}

	mutex_unlock(&memory_list_mutex);
	return -ESRCH;
}



void print_memory_limitation_list(void) {
	struct memory_limit_entry * entry;

	mutex_lock(&memory_list_mutex);
	printk("-----------------------------------------------\n");
	printk("Printing processes that have memory limitation:\n");
	list_for_each_entry(entry, &memory_list, list) {
		printk(KERN_INFO "PID: %d, memory_limit: %zu\n", entry->data.pid, entry->data.memory_limit);
	}
	printk("-----------------------------------------------\n");
	mutex_unlock(&memory_list_mutex);
}
