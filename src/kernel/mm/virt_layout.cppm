export module kernel.mm:layout;

import :types;
import kernel.base;

export namespace mm {

inline constexpr base::size_t KERN_DIRECT_MAP_REGION_BASE   = 0xFFFF800000000000;
inline constexpr base::size_t KERN_DIRECT_MAP_REGION_END    = 0xFFFFBFFFFFFFFFFF;

inline constexpr base::size_t KERN_DYNAMIC_MAP_REGION_BASE  = 0xFFFFC00000000000;
inline constexpr base::size_t KERN_DYNAMIC_MAP_REGION_END   = 0xFFFFCFFFFFFFFFFF;

inline constexpr base::size_t KERN_PAGE_DATABASE_REGION_BASE= 0xFFFFF00000000000;
inline constexpr base::size_t KERN_PAGE_DATABASE_REGION_END = 0xFFFFF7FFFFFFFFFF;

inline constexpr base::size_t KERN_STACK_REGION_BASE        = 0xFFFFFA0000000000;
inline constexpr base::size_t KERN_STACK_REGION_END         = 0xFFFFFA0FFFFFFFFF;

inline constexpr base::size_t KERN_SEG_TEXT_REGION_START    = 0xFFFFFF8000000000;
inline constexpr base::size_t KERN_SEG_TEXT_REGION_END      = 0xFFFFFF8007FFFFFF;

inline constexpr base::size_t KERN_SEG_DATA_REGION_START    = 0xFFFFFF8008000000;
inline constexpr base::size_t KERN_SEG_DATA_REGION_END      = 0xFFFFFF800FFFFFFF;

inline constexpr base::size_t KERN_SEG_RODATA_REGION_START  = 0xFFFFFF8010000000;
inline constexpr base::size_t KERN_SEG_RODATA_REGION_END    = 0xFFFFFF8017FFFFFF;

inline constexpr base::size_t KERN_SEG_BSS_REGION_START     = 0xFFFFFF8018000000;
inline constexpr base::size_t KERN_SEG_BSS_REGION_END       = 0xFFFFFF801FFFFFFF;

virt_addr_t physmem_base, vmremap_base, kernel_base;

};
