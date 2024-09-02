#include <closureos/cpp_base.hpp>
#include <closureos/lock.h>
#include <closureos/errno.h>

extern "C" {
void* __dso_handle __attribute__((visibility("hidden")));

struct dtor_info *global_dtors = nullptr;

int __cxa_atexit(void (*destructor) (void *), void *arg, void *__dso_handle)
{
    struct dtor_info *new_info;
    
    new_info = new struct dtor_info;
    if (new_info == nullptr) {
        return -ENOMEM;
    }

    new_info->next = global_dtors;
    new_info->destructor = destructor;
    new_info->arg = arg;
    new_info->__dso_handle = __dso_handle;

    global_dtors = new_info;

    return 0;
}

spinlock_t dtor_exit_lock;

void __cxa_finalize(void* dso_handle)
{
    struct dtor_info **pdtor = &global_dtors;

    spin_lock(&dtor_exit_lock);

    while (*pdtor) {
        struct dtor_info *dtor = *pdtor;

        if (!dso_handle || dtor->__dso_handle == dso_handle) {
            *pdtor = dtor->next;
            dtor->destructor(dtor->arg);
            delete dtor;
        } else {
            pdtor = &dtor->next;
        }
    }

    spin_unlock(&dtor_exit_lock);
}

void __cxa_pure_virtual()
{
    /**
     * TODO: add error message output
    */
}

} // extern "C"

void* operator new(size_t sz)
{
    return kmalloc(sz);
}

void* operator new[](size_t sz)
{
    return kmalloc(sz);
}

void operator delete(void *p)
{
    kfree(p);
}

void operator delete[](void *p)
{
    kfree(p);
}

void operator delete(void *p, unsigned long sz)
{
    kfree(p);
}
