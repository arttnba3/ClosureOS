export module kernel.lib.list;

import kernel.lib.container;

#include <closureos/types.h>
#include <closureos/compiler.h>

export namespace lib {

struct ListHead {
    ListHead *prev, *next;
};

template <typename HeadType>
auto __always_inline list_head_init(HeadType head) -> void
{
    head->prev = head;
    head->next = head;
}

template <typename NodeType, typename PrevType, typename NextType>
auto __always_inline list_add(NodeType node, PrevType prev, NextType next) -> void
{
    node->prev = prev;
    prev->next = node;
    node->next = next;
    next->prev = node;
}

/* add a new node to the list as entry->prev */
auto __always_inline list_add_prev(ListHead *entry, ListHead *prev) -> void
{
    entry->prev->next = prev;
    prev->prev = entry->prev;
    prev->next = entry;
    entry->prev = prev;
}

/* add a new node to the list as entry->next */
auto __always_inline list_add_next(ListHead *entry, ListHead *next) -> void
{
    entry->next->prev = next;
    next->prev = entry;
    next->next = entry->next;
    entry->next = next;
}

template <typename NodeType>
auto __always_inline list_del(NodeType node) -> void
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

template <typename EntryType>
auto __always_inline list_empty(EntryType entry) -> bool
{
    return (entry->next == entry) && (entry->prev == entry);
}

template <typename ListPtrType, typename ContainerType, typename MemberType>
auto __always_inline list_entry(ListPtrType *ptr, const MemberType ContainerType::* member) -> ContainerType*
{
    return container_of(ptr, member);
}

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


};
