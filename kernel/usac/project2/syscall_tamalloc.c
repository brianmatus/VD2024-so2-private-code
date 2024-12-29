#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include <linux/mman.h>

SYSCALL_DEFINE1(matus_tamalloc, size_t, size)
{
	unsigned long addr;
	size_t aligned_size;

	if (size == 0) {
		return -EINVAL;
	}

	aligned_size = PAGE_ALIGN(size);
	if (!aligned_size) {
		return -ENOMEM;
	}

	//Para crear la paginacion en zeros de manera dinamica, necesitamos un mapeo virtual
	//Que caracteristicas tienen que tener este mapeo?
	//El mapeo debe asegurar que sean 0's
	//El mapeo no debe reservar memoria fisica en paginas al momento de inicializar,
	//solamente hasta su primer acceso (escritura) (La lectura no causara fallo de pagina debido a CoW)

	addr = vm_mmap(NULL, 0, aligned_size,
		PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, 0);

	if (IS_ERR_VALUE(addr)) {
		printk(KERN_ERR "ERROR AL MAPEAR MEMORIA VIRTUAL CON TAMALLOC :( ya no sale");
		return addr;
	}
	return addr;
}