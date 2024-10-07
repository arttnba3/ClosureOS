export module kernel.lib.atomic;

import kernel.base;

#include <closureos/compiler.h>

export namespace lib::atomic {

typedef volatile base::int32_t atomic_t;

template <typename PtrType, typename OldValType, typename NewValType>
__always_inline auto atomic_compare_and_swap(PtrType ptr, OldValType oldval, NewValType newval) -> bool
{
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

template <typename PtrType, typename NewValType>
__always_inline auto atomic_set(PtrType ptr, NewValType newval) -> auto
{
    return __sync_lock_test_and_set(ptr, newval);
}

template <typename PtrType>
__always_inline auto atomic_inc(PtrType ptr) -> auto
{
    return __sync_fetch_and_add(ptr, 1);
}

template <typename PtrType>
__always_inline auto atomic_dec(PtrType ptr) -> auto
{
    return __sync_fetch_and_sub(ptr, 1);
}

template <typename PtrType>
__always_inline auto atomic_read(PtrType ptr) -> auto
{
    return __sync_fetch_and_add(ptr, 0);
}

class SpinLock {
public:
    SpinLock();
    ~SpinLock();

    auto Lock(void) -> void;
    auto TryLock(void) -> bool;
    auto UnLock(void) -> void;
    auto Reset(void) -> void;

private:
    atomic_t counter;
};

};
