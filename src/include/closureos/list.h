#ifndef CLOSUREOS_LIST_H
#define CLOSUREOS_LIST_H

#include <closureos/container.h>
#include <closureos/compiler.h>

struct list_head {
    struct list_head *prev, *next;
};

/* define a variable as head node of a list */
#define LIST_HEAD(name)             \
        struct list_head name = {   \
            .next = &name,          \
            .prev = &name,          \
        }

#define STATIC_LIST_HEAD(name)              \
        static struct list_head name = {    \
            .next = &name,                  \
            .prev = &name,                  \
        }

#define list_head_init(head)            \
        do {                            \
            (head)->prev = head;        \
            (head)->next = head;        \
        } while(0)

#define list_add(node, prev, next)      \
        {                               \
            (node)->prev = prev;        \
            prev->next = (node);        \
            (node)->next = next;        \
            next->prev = (node);        \
        }

/* add a new node to the list as entry->prev */
static __always_inline void list_add_prev(struct list_head *entry, 
                                          struct list_head *prev)
{
    entry->prev->next = prev;
    prev->prev = entry->prev;
    prev->next = entry;
    entry->prev = prev;
}

/* add a new node to the list as entry->next */
static __always_inline void list_add_next(struct list_head *entry, 
                                          struct list_head *next)
{
    entry->next->prev = next;
    next->prev = entry;
    next->next = entry->next;
    entry->next = next;
}

#define list_del(node)                                  \
        do {                                            \
            (node)->prev->next = (node)->next;          \
            (node)->next->prev = (node)->prev;          \
        } while (0)

#define list_empty(entry)       \
        (((entry)->next == entry) && ((entry)->prev == entry))

#define list_entry(ptr, type, member)       \
        container_of(ptr, type, member)

#define list_head_first_entry(ptr, type, member)    \
        list_entry((ptr)->next, type, member)

#define list_next_entry(ptr, member)      \
        container_of(ptr->member.next, typeof(*ptr), member)

#define list_entry_is_head(ptr, head, member)       \
        ((&ptr->member) == (head))

/**
 * Iterate each item of a given list head, not suitable for headless list
 * @pos:    the pointer defined by you to use as the iterator
 * @head:   pointer of the head node (sentinel) of your linked list
 * @member: the member within specific struct 
*/
#define list_head_for_each_entry(pos, head, member)                     \
        for (pos = list_head_first_entry(head, typeof(*pos), member);   \
            list_entry_is_head(pos, head, member);                      \
            pos = list_next_entry(pos, member))


#endif // CLOSUREOS_LIST_H
