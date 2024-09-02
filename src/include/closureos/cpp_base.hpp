#ifndef CLOSUREOS_CPP_BASE_HPP
#define CLOSUREOS_CPP_BASE_HPP

#include <closureos/types.h>
#include <closureos/compiler.h>

/* memory management */

extern "C" {
    void *kmalloc(size_t sz);
    void kfree(void* object);
}

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

/* global constructor/destructor */

struct dtor_info {
    struct dtor_info *next;
    void (*destructor) (void *);
    void *arg;
    void *__dso_handle;
};

extern "C" {
    extern "C" int __cxa_atexit ( void (*f)(void *), void *p, void *d );
    extern void __cxa_finalize(void* dso_handle);
    extern void* __dso_handle __attribute__((visibility("hidden")));
}

/* virtual functions */

extern "C" {
    extern void __cxa_pure_virtual();
}

#endif // CLOSUREOS_CPP_BASE_HPP
