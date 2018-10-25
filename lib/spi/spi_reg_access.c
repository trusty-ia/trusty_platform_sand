/*******************************************************************************
* Copyright (c) 2016 Intel Corporation
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

#include <errno.h>
#include <stdint.h>
#include <platform/sand.h>
#include <platform/pci_config.h>
#include <platform/lpss_spi.h>
#include <printf.h>
#include <debug.h>
#include <arch/x86.h>
#include <arch/x86/mmu.h>
#include <arch/mmu.h>
#include <kernel/vm.h>

uint64_t spi_mmio_base_addr = 0;

void spi_mmu_init(void)
{
    uint64_t io_base = 0;
    arch_flags_t access = ARCH_MMU_FLAG_PERM_NO_EXECUTE |
        ARCH_MMU_FLAG_UNCACHED | ARCH_MMU_FLAG_PERM_USER;
    struct map_range range;
    map_addr_t pml4_table = (map_addr_t)paddr_to_kvaddr(x86_get_cr3());

    io_base = (uint64_t)(pci_read32(SPI_BUS, SPI_DEV, SPI_FUN, 0x10) & ~0xF);

    range.start_vaddr = (map_addr_t)(0xFFFFFFFF00000000ULL + (uint64_t)io_base);
    range.start_paddr = (map_addr_t)io_base;
    range.size = PAGE_SIZE;
    x86_mmu_map_range(pml4_table, &range, access);

    spi_mmio_base_addr = range.start_vaddr;
}

static inline uint32_t __raw_read32(uint64_t addr)
{
    return *(volatile uint32_t *)addr;
}

static inline void __raw_write32(uint64_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}

uint64_t get_spi_mmio_base(void)
{
    uint64_t io_base = spi_mmio_base_addr;

    return io_base;
}


uint32_t lpss_spi_read(uint32_t reg)
{
    if (!spi_mmio_base_addr) {
     spi_mmio_base_addr = get_spi_mmio_base();
    }
    return __raw_read32(spi_mmio_base_addr + (uint64_t)reg);
}

void lpss_spi_write(uint32_t reg, uint32_t val)
{
    if (!spi_mmio_base_addr) {
        spi_mmio_base_addr = get_spi_mmio_base();
    }
    __raw_write32(spi_mmio_base_addr + (uint64_t)reg, val);
}
