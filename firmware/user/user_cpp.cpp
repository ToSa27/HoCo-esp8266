#include <user_cpp.h>

extern "C" {
#include <user_interface.h>
#include <debug.h>
}

void *operator new(size_t size)
{
	DEBUG("new: %d (%d)", size, system_get_free_heap_size());
	return os_malloc(size);
}

void *operator new[](size_t size)
{
	DEBUG("new[]: %d (%d)", size, system_get_free_heap_size());
	return os_malloc(size);
}

void operator delete(void * ptr)
{
	if (ptr != NULL)
		os_free(ptr);
	DEBUG("delete: (%d)", system_get_free_heap_size());
}

void operator delete[](void * ptr)
{
	if (ptr != NULL)
		os_free(ptr);
	DEBUG("delete[]: (%d)", system_get_free_heap_size());
}

extern "C" void __cxa_pure_virtual(void) __attribute__ ((__noreturn__));
extern "C" void __cxa_deleted_virtual(void) __attribute__ ((__noreturn__));
extern "C" void abort()
{
   while(true); // enter an infinite loop and get reset by the WDT
}

void __cxa_pure_virtual(void) {
  abort();
}

void __cxa_deleted_virtual(void) {
  abort();
}

extern void (*__init_array_start)(void);
extern void (*__init_array_end)(void);

void do_global_ctors(void)
{
   void (**p)(void);
   for (p = &__init_array_start; p != &__init_array_end; ++p)
   {
      (*p)();
   }
}
