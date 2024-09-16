#ifndef CLOSUREOS_CONTAINER_H
#define CLOSUREOS_CONTAINER_H

#ifndef NULL
#define NULL ((void*) 0)
#endif

/* offset of a member in a type, calculated by compiler */
#define builtin_offsetof(type, member) __builtin_offsetof(type, member)

#define offsetof(type, member) ({   \
    type *ptr = (type*) NULL;       \
    (size_t) &(ptr->member);        \
})

/**
 * Cast a pointer of a given container member to the container type pointer
*/
#define container_of(ptr, type, member) ({                      \
    (type*) ((size_t) ptr - builtin_offsetof(type, member));    \
})

#endif // CLOSUREOS_CONTAINER_H
