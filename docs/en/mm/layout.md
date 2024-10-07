# Memory layout

Memory layout of ClosureOS is defined as below:

| Start address | End address | Size | Description |
| :------------:|:-----------:|:----:|:-----------:|
| 0x0000000000000000 | 0x00007FFFFFFFFFFF | 128TB | memory space for user-mode process, isolate for per one |
| 0x0000800000000000 | 0xFFFF7FFFFFFFFFFF | 16776960 TB | unused hole |
| | | | shared kernel-space virtual memory for all processes |
| 0xFFFF800000000000 | 0xFFFFBFFFFFFFFFFF | 64TB | direct mapping of first 64TB physical memory (physmem_base) |
| 0xFFFFC00000000000 | 0xFFFFCFFFFFFFFFFF | 16TB | dynamic kernel memory mapping region (vmremap_base) |
| 0xFFFFD00000000000 | 0xFFFFEFFFFFFFFFFF | 32TB | unused hole |
| 0xFFFFF00000000000 | 0xFFFFF7FFFFFFFFFF | 8TB | page database (pgdb_base) |
| 0xFFFFF80000000000 | 0xFFFFF9FFFFFFFFFF | 2TB | unused hole |
| 0xFFFFFA0000000000 | 0xFFFFFA0FFFFFFFFF | 64GB | kernel stack, isolate for each process |
| 0xFFFFFA1000000000 | 0xFFFFFF7FFFFFFFFF | 5568GB | unused hole |
| 0xFFFFFF8000000000 | 0xFFFFFF800FFFFFFF | 256MB | kernel .text segment |
| 0xFFFFFF8010000000 | 0xFFFFFF801FFFFFFF | 256MB | kernel .data segment |
| 0xFFFFFF8020000000 | 0xFFFFFF802FFFFFFF | 256MB | kernel .rodata segment |
| 0xFFFFFF8030000000 | 0xFFFFFF803FFFFFFF | 256MB | kernel .bss segment |
| 0xFFFFFF8050000000 | 0xFFFFFFFFFFFFFFFF | 511GB | unused hole |
