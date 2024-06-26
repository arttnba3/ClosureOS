#ifndef MM_LAYOUT_H
#define MM_LAYOUT_H

#define KERN_DIRECT_MAP_REGION_BASE     0xFFFF800000000000
#define KERN_DIRECT_MAP_REGION_END      0xFFFFBFFFFFFFFFFF

#define KERN_DYNAMIC_MAP_REGION_BASE    0xFFFFC00000000000
#define KERN_DYNAMIC_MAP_REGION_END     0xFFFFCFFFFFFFFFFF

#define KERN_PAGE_DATABASE_REGION_BASE  0xFFFFF00000000000
#define KERN_PAGE_DATABASE_REGION_END   0xFFFFF7FFFFFFFFFF

#define KERN_STACK_REGION_BASE          0xFFFFFA0000000000
#define KERN_STACK_REGION_END           0xFFFFFA0FFFFFFFFF

#define KERN_SEG_TEXT_REGION_START      0xFFFFFF8000000000
#define KERN_SEG_TEXT_REGION_END        0xFFFFFF8007FFFFFF

#define KERN_SEG_DATA_REGION_START      0xFFFFFF8008000000
#define KERN_SEG_DATA_REGION_END        0xFFFFFF800FFFFFFF

#define KERN_SEG_RODATA_REGION_START    0xFFFFFF8010000000
#define KERN_SEG_RODATA_REGION_END      0xFFFFFF8017FFFFFF

#define KERN_SEG_BSS_REGION_START       0xFFFFFF8018000000
#define KERN_SEG_BSS_REGION_END         0xFFFFFF801FFFFFFF

#endif // MM_LAYOUT_H
