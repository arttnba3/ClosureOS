module kernel.lib.atomic;

namespace lib::atomic {

#define SPINLOCK_LOCKED 1
#define SPINLOCK_FREE 0

SpinLock::SpinLock()
{
    this->Reset();
}

SpinLock::~SpinLock()
{

}

auto SpinLock::Lock(void) -> void
{
    while (!atomic_compare_and_swap(&this->counter, SPINLOCK_FREE, SPINLOCK_LOCKED)) {
        /* infinite waiting looooop... */
    }
}

auto SpinLock::TryLock(void) -> bool
{
    return atomic_compare_and_swap(&this->counter, SPINLOCK_FREE, SPINLOCK_LOCKED);
}

auto SpinLock::UnLock(void) -> void
{
    atomic_set(&this->counter, SPINLOCK_FREE);
}

auto SpinLock::Reset(void) -> void
{
    atomic_set(&this->counter, SPINLOCK_FREE);
}

};
