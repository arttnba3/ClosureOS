#ifndef CLOSUREOS_LOCK_H
#define CLOSUREOS_LOCK_H

#include <closureos/types.h>
#include <closureos/atomic.h>

#define LOCK_USED 1
#define LOCK_FREE 0

typedef struct {
    atomic_t counter;
} spinlock_t;

static inline void spin_lock_init(spinlock_t *lock)
{
    atomic_set(&lock->counter, LOCK_FREE);
}

static inline void spin_lock(spinlock_t *lock)
{
    while (!atomic_compare_and_swap(&lock->counter, LOCK_FREE, LOCK_USED)) {
        continue;
    }
}

static inline void spin_unlock(spinlock_t *lock)
{
    while (!atomic_compare_and_swap(&lock->counter, LOCK_USED, LOCK_FREE)) {
        continue;
    }
}

#endif // CLOSUREOS_LOCK_H
