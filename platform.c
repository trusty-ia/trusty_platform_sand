/*******************************************************************************
 * Copyright (c) 2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/
#include <err.h>
#include <arch/x86/mmu.h>
#include <platform.h>
#include <platform/sand.h>
#include <platform/multiboot.h>
#include <arch/x86.h>
#include <arch/mmu.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <kernel/vm.h>
#include "mem_map.h"
#include <lib/trusty/trusty_device_info.h>

extern int _end_of_ram;

#ifdef WITH_KERNEL_VM
extern int _end;
static uintptr_t _heap_start = (uintptr_t) &_end;
static uintptr_t _heap_end = (uintptr_t) &_end;
#else
extern uintptr_t _heap_end;
#endif
extern uint64_t __code_start;
extern uint64_t __code_end;
extern uint64_t __rodata_start;
extern uint64_t __rodata_end;
extern uint64_t __data_start;
extern uint64_t __data_end;
extern uint64_t __bss_start;
extern uint64_t __bss_end;
extern void pci_init(void);
extern void arch_mmu_init_percpu(void);

/* Address width */
uint32_t g_addr_width;

/* Kernel global CR3 */
map_addr_t g_CR3 = 0;

#ifdef WITH_KERNEL_VM
struct mmu_initial_mapping mmu_initial_mappings[] = {
    {.phys = TRUSTY_START_ADDR,
        .virt = TRUSTY_START_ADDR,
        .size = TARGET_MAX_MEM_SIZE - TRUSTY_START_ADDR,
        .flags = 0,
        .name = "memory"},
    /* 1 GB of memory */
    {.phys = 0,
        .virt = 0,
        .size = 1024 * 1024 * 1024,
        .flags = MMU_INITIAL_MAPPING_TEMPORARY},
    /* null entry to terminate the list */
    {0}
};
#endif

void platform_init_mmu_mappings(void)
{
    struct map_range range;
    arch_flags_t access;
    map_addr_t *init_table, phy_init_table;

    /* getting the address width from CPUID instr */
    g_addr_width = x86_get_address_width();

    /* Creating the First page in the page table hirerachy */
    /* Can be pml4, pdpt or pdt based on x86_64, x86 PAE mode & x86 non-PAE mode respectively */
    init_table = memalign(PAGE_SIZE, PAGE_SIZE);
    ASSERT(init_table);
    memset(init_table, 0, PAGE_SIZE);

    phy_init_table = (map_addr_t) X86_VIRT_TO_PHYS(init_table);

    /* kernel code section mapping */
    access = ARCH_MMU_FLAG_PERM_RO;
    range.start_vaddr = range.start_paddr = (map_addr_t) & __code_start;
    range.size =
        ((map_addr_t) & __code_end) - ((map_addr_t) & __code_start);
    x86_mmu_map_range(phy_init_table, &range, access);

    /* kernel data section mapping */
    access = 0;
#if defined(ARCH_X86_64) || defined(PAE_MODE_ENABLED)
    access |= ARCH_MMU_FLAG_PERM_NO_EXECUTE;
#endif
    range.start_vaddr = range.start_paddr = (map_addr_t) & __data_start;
    range.size =
        ((map_addr_t) & __data_end) - ((map_addr_t) & __data_start);
    x86_mmu_map_range(phy_init_table, &range, access);

    /* kernel rodata section mapping */
    access = ARCH_MMU_FLAG_PERM_RO;
#if defined(ARCH_X86_64) || defined(PAE_MODE_ENABLED)
    access |= ARCH_MMU_FLAG_PERM_NO_EXECUTE;
#endif
    range.start_vaddr = range.start_paddr = (map_addr_t) & __rodata_start;
    range.size =
        ((map_addr_t) & __rodata_end) - ((map_addr_t) & __rodata_start);
    x86_mmu_map_range(phy_init_table, &range, access);

    /* kernel bss section and kernel heap mappings */
    access = 0;
#ifdef ARCH_X86_64
    access |= ARCH_MMU_FLAG_PERM_NO_EXECUTE;
#endif
    range.start_vaddr = range.start_paddr = (map_addr_t) & __bss_start;
    range.size = ((map_addr_t) _heap_end) - ((map_addr_t) & __bss_start);
    x86_mmu_map_range(phy_init_table, &range, access);

    /* Mapping for BIOS, devices */
    access = 0;
    range.start_vaddr = range.start_paddr = (map_addr_t) 0;
    range.size = ((map_addr_t) & __code_start);
    x86_mmu_map_range(phy_init_table, &range, access);

    /* Mapping upper boundary to target maxium memory size */
    access = ARCH_MMU_FLAG_PERM_NO_EXECUTE;
    range.start_vaddr = range.start_paddr = (map_addr_t)_heap_end;
    range.size = ((map_addr_t)TARGET_MAX_MEM_SIZE - (map_addr_t)_heap_end);
    x86_mmu_map_range(phy_init_table, &range, access);

    /* Moving to the new CR3 */
    g_CR3 = (map_addr_t) phy_init_table;
    x86_set_cr3((map_addr_t) phy_init_table);
}

#ifdef WITH_KERNEL_VM
static pmm_arena_t heap_arena = {
    .name = "heap",
    .base = 0,
    .size = 0,
    .priority = 1,
    .flags = PMM_ARENA_FLAG_KMAP
};

void heap_arena_init()
{
    uintptr_t heap_base = ((uintptr_t) _heap_start);
    uintptr_t heap_size = (uintptr_t) _heap_end - (uintptr_t) _heap_start;

    heap_arena.base = PAGE_ALIGN(heap_base);
    heap_arena.size = PAGE_ALIGN(heap_size);
}
#endif

void clear_sensitive_data(void)
{
    trusty_device_info_t *   dev_info = NULL;

    if(g_trusty_startup_info && g_trusty_startup_info->size_of_this_struct > 0) {
        dev_info = (trusty_device_info_t *)g_trusty_startup_info->trusty_mem_base;
        /* clear the trusty_device_info_t */
        if(dev_info && dev_info->size > 0)
            memset(dev_info, 0, dev_info->size);

        /* clear the g_trusty_startup_info */
        memset(g_trusty_startup_info, 0, g_trusty_startup_info->size_of_this_struct);
        g_trusty_startup_info = NULL;
    }
}

/*
* TODO: need to enhance the panic handler
* currently, if we got panic in boot stage, the behavior
* is not expect, it will failed with SMC call since Android
* not started yet.
*/
void platform_heap_init(void)
{
    if(!g_trusty_startup_info)
        panic("g_trusty_startup_info is NULL!\n");

    if(g_trusty_startup_info->size_of_this_struct != sizeof(trusty_startup_info_t))
        panic("trusty startup structure mismatch!\n");

    /* update the heap with the real size passed from vmm */
    _heap_end = (uintptr_t)
        (&_end + g_trusty_startup_info->heap_size_in_mb MEGABYTES);
}

void platform_early_init(void)
{
    /* initialize the interrupt controller */
    platform_init_interrupts();

    /* initialize the timer */
    platform_init_timer();

    /* initialize the heap */
    platform_heap_init();

#ifdef WITH_KERNEL_VM
    heap_arena_init();
    pmm_add_arena(&heap_arena);
#endif
}

void platform_init(void)
{

    /* MMU init for x86 Archs done after the heap is setup */
    arch_mmu_init_percpu();
    platform_init_mmu_mappings();
}
