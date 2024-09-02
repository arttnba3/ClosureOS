#ifndef CLOSUREOS_CPP_BASE_HPP
#define CLOSUREOS_CPP_BASE_HPP

#include <closureos/types.h>
#include <closureos/lock.h>
#include <closureos/compiler.h>

/* memory management */

extern "C" {
    void *kmalloc(size_t sz);
    void kfree(void* object);
}

extern void* operator new(size_t sz);

extern void* operator new[](size_t sz);

extern void operator delete(void *p);

extern void operator delete[](void *p);

extern void operator delete(void *p, unsigned long sz);

/* global constructor/destructor */

struct dtor_info {
    struct dtor_info *next;
    void (*destructor) (void *);
    void *arg;
    void *__dso_handle;
};

extern "C" {
    extern int (*__init_array)(void);
    extern spinlock_t dtor_exit_lock;
    extern int __cxa_atexit ( void (*f)(void *), void *p, void *d );
    extern void __cxa_finalize(void* dso_handle);
    extern void* __dso_handle __attribute__((visibility("hidden")));
}

/* virtual functions */

extern "C" {
    extern void __cxa_pure_virtual();
}

#endif // CLOSUREOS_CPP_BASE_HPP
