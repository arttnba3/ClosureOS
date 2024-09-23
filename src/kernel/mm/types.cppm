export module kernel.mm:types;

import kernel.lib;

export namespace mm {

typedef lib::size_t virt_addr_t;
typedef lib::size_t phys_addr_t;
typedef lib::size_t dma_addr_t;
typedef lib::size_t pfn_t;

inline constexpr lib::size_t PAGE_SHIFT = 12;
inline constexpr lib::size_t PAGE_SIZE  = (1UL << PAGE_SHIFT);
inline constexpr lib::size_t PAGE_MASK  = (~(PAGE_SIZE - 1));

};
