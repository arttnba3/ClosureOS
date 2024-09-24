#include <closureos/cpp_base.hpp>
#include <closureos/errno.h>

import kernel.lib;
import kernel.mm;

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

lib::atomic::SpinLock dtor_exit_lock;

void __cxa_finalize(void* dso_handle)
{
    struct dtor_info **pdtor = &global_dtors;

    dtor_exit_lock.Lock();

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

    dtor_exit_lock.UnLock();
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
    return mm::GloblKHeapPool->Malloc(sz);
}

void* operator new[](size_t sz)
{
    return mm::GloblKHeapPool->Malloc(sz);
}

void operator delete(void *p) noexcept
{
    mm::GloblKHeapPool->Free(p);
}

void operator delete[](void *p) noexcept
{
    mm::GloblKHeapPool->Free(p);
}

void operator delete(void *p, unsigned long sz) noexcept
{
    mm::GloblKHeapPool->Free(p);
}

void operator delete[](void *p, unsigned long sz) noexcept
{
    mm::GloblKHeapPool->Free(p);
}
