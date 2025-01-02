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


bool is_request_accepted(pid_t pid, size_t requested_memory) {
    struct memory_limit_entry *entry;
    struct task_struct *task;
    size_t current_usage;

    task = find_task_by_vpid(pid);
    if (!task) {
    	printk(KERN_ERR "INVALID PID SEARCH? PROCESS DIED AFTER REQUESTING MEMORY????");
        return false; //Should not happen?
    }

    if (!task->mm) {
    	printk(KERN_ERR "INVALID PID total_vm VALUE???? SHOULD NOT HAPPEN?");
        return false; //Should not happen?
    }

    current_usage = (task->mm->total_vm << PAGE_SHIFT);

    mutex_lock(&memory_list_mutex);
    list_for_each_entry(entry, &memory_list, list) {
        if (entry->data.pid == pid) {
            if (current_usage + requested_memory > entry->data.memory_limit) {
                mutex_unlock(&memory_list_mutex);
                printk(KERN_INFO "USAC-Project3-> Denied call for PID %d\n", pid);
                return false;
            }
            //Accept request
            mutex_unlock(&memory_list_mutex);
            printk(KERN_INFO "USAC-Project3-> Allowed call for PID %d, total_vm is now %zu\n", pid, current_usage + requested_memory);
            return true;
            break;
        }
    }
    mutex_unlock(&memory_list_mutex);
    return true;
}


SYSCALL_DEFINE2(matus_add_memory_limit, pid_t, process_pid, size_t, memory_limit)
{
	struct task_struct *task;
	struct memory_limit_entry *entry;

	if (process_pid < 0) {
        return -EINVAL;
    }

    task = find_task_by_vpid(process_pid);
    if (!task) {
        return -ESRCH;
    }

    if (!capable(CAP_SYS_ADMIN)) {
        return -EPERM;
    }

    if ((long)memory_limit < 0) {
        return -EINVAL;
    }


    //Este requerimiento es un poco distinto de lo que se esperaria.
    //Para futuras versiones, seria mejor poder limitar procesos aunque se pasen de su uso actual
    //Para que el proceso solo sea capaz de liberar memoria pero no solicitar nueva
    //Por otro lado, 
	size_t current_usage = (task->mm->total_vm << PAGE_SHIFT);
	if (memory_limit < current_usage) {
        return -100; // Error: el nuevo límite es menor al consumo actual
    }

    mutex_lock(&memory_list_mutex);

    // Validar si el proceso ya está en la lista
    struct memory_limit_entry *tmp;
    list_for_each_entry(tmp, &memory_list, list) {
        if (tmp->data.pid == process_pid) {
            mutex_unlock(&memory_list_mutex);
            return -101;
        }
    }

	entry = kmalloc(sizeof(struct memory_limit_entry), GFP_KERNEL);
	if (!entry) {
		mutex_unlock(&memory_list_mutex);
		return -ENOMEM;
	}

	entry->data.pid = process_pid;
	entry->data.memory_limit = memory_limit;
	list_add(&entry->list, &memory_list);

	mutex_unlock(&memory_list_mutex);
	return 0;
}


SYSCALL_DEFINE3(matus_get_memory_limits, struct memory_limitation*, u_processes_buffer, size_t, max_entries, int*, processes_returned)
{
	struct memory_limit_entry * entry;
	int count = 0;

	if (!u_processes_buffer || !processes_returned) {
        return -EINVAL; //Validar el puntero de user-space
    }

    if (max_entries <= 0) {
        return -EINVAL;
    }

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
	// print_memory_limitation_list();
	return 0;
}


SYSCALL_DEFINE2(matus_update_memory_limit, pid_t, process_pid, size_t, memory_limit)
{
	struct memory_limit_entry * entry;

	if (!capable(CAP_SYS_ADMIN)) {
        return -EPERM;
    }

    if (process_pid < 0) {
        return -EINVAL;
    }

    if (memory_limit < 0) {
        return -EINVAL;
    }


	mutex_lock(&memory_list_mutex);
	list_for_each_entry(entry, &memory_list, list) {
		if (entry->data.pid == process_pid) {
			entry->data.memory_limit = memory_limit;
			mutex_unlock(&memory_list_mutex);
			return 0;
		}
	}
	mutex_unlock(&memory_list_mutex);
	return -102; //Process not on list
}


SYSCALL_DEFINE1(matus_remove_memory_limit, pid_t, process_pid)
{
	struct memory_limit_entry * entry, * tmp;

	if (!capable(CAP_SYS_ADMIN)) {
        return -EPERM;
    }

    if (process_pid < 0) {
        return -EINVAL;
    }

	mutex_lock(&memory_list_mutex);

	//FIXME change to only list_for_each_entry
	list_for_each_entry_safe(entry, tmp, &memory_list, list) {
		if (entry->data.pid == process_pid) {
			list_del(&entry->list);
			kfree(entry);
			mutex_unlock(&memory_list_mutex);
			// print_memory_limitation_list();
			return 0;
		}
	}

	mutex_unlock(&memory_list_mutex);
	return -102; //Process not in list
}



// void print_memory_limitation_list(void) {
// 	struct memory_limit_entry * entry;

// 	mutex_lock(&memory_list_mutex);
// 	printk("-----------------------------------------------\n");
// 	printk("Printing processes that have memory limitation:\n");
// 	list_for_each_entry(entry, &memory_list, list) {
// 		printk(KERN_INFO "PID: %d, memory_limit: %zu\n", entry->data.pid, entry->data.memory_limit);
// 	}
// 	printk("-----------------------------------------------\n");
// 	mutex_unlock(&memory_list_mutex);
// }
