/*
    GNU linker script for NRF52832
*/

_flash_size = 0x78000; /* bootloader starts at 0x78000 */
_ram_size   = 64K;
_fs_size    = 0;

/* produce a link error if there is not this amount of RAM for these sections */
_stack_size = 8K;
_minimum_heap_size = 32K;
