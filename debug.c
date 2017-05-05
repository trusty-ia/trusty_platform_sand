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
#include <stdarg.h>
#include <reg.h>
#include <stdio.h>
#include <kernel/thread.h>
#include <arch/x86.h>
#include <lib/cbuf.h>
#include <platform/interrupts.h>
#include <platform/debug.h>
#include <platform/sand.h>
#include <string.h>
#include <kernel/vm.h>
#include <kernel/mutex.h>

#define BUF_SIZE 4096
static char uart_early_buf[BUF_SIZE];
static int put_idx = 0, get_idx = 0;
static mutex_t uart_lock = MUTEX_INITIAL_VALUE(uart_lock);



extern having_print_cb;
uint8_t (*io_get_reg)(uint64_t base_addr, uint32_t reg_id);
void (*io_set_reg)(uint64_t base_addr, uint32_t reg_id, uint8_t val);

#if PRINT_USE_MMIO
static uint64_t mmio_base_addr = 0;

static uint8_t uart_mmio_get_reg(uint64_t base_addr, uint32_t reg_id)
{
    return *(volatile uint8_t *)(base_addr + (uint64_t)reg_id * 4);
}

static void uart_mmio_set_reg(uint64_t base_addr, uint32_t reg_id, uint8_t val)
{
    *(volatile uint8_t *)(base_addr + (uint64_t)reg_id * 4) = val;
}

uint64_t get_mmio_base(void)
{
    return mmio_base_addr;
}
#elif PRINT_USE_IO_PORT
inline uint8_t asm_in8(uint16_t port)
{
    uint8_t val8;

    __asm__ __volatile__ (
            "inb %1, %0"
            : "=a" (val8)
            : "d" (port));
    return val8;
}

inline void asm_out8(uint16_t port, uint8_t val8)
{
    __asm__ __volatile__ (
            "outb %1, %0"
            :
            : "d" (port), "a" (val8));

}

static uint8_t serial_io_get_reg(uint64_t base_addr, uint32_t reg_id)
{
    return asm_in8((uint16_t)base_addr + (uint16_t)reg_id);
}

static void serial_io_set_reg(uint64_t base_addr, uint32_t reg_id, uint8_t val)
{
    asm_out8((uint16_t)base_addr + (uint16_t)reg_id, val);
}
#endif

static void uart_putc(char c)
{
    uart_lsr_t lsr;
    uint64_t io_base;

#if PRINT_USE_MMIO
    io_base = mmio_base_addr;
    io_get_reg = uart_mmio_get_reg;
    io_set_reg = uart_mmio_set_reg;
#elif PRINT_USE_IO_PORT
    io_get_reg = serial_io_get_reg;
    io_set_reg = serial_io_set_reg;
    io_base = TARGET_SERIAL_IO_BASE;
#else
    return;
#endif

    mutex_acquire(&uart_lock);

    uart_early_buf[put_idx++] = c;
    put_idx %= BUF_SIZE;
    if (put_idx == get_idx) {
        get_idx++;
        get_idx %= BUF_SIZE;
    }

#if PRINT_USE_MMIO
    if (!mmio_base_addr) {
        mutex_release(&uart_lock);
        return;
    }
#endif

    while (put_idx != get_idx) {
        while (1) {
            lsr.data = io_get_reg(io_base, UART_REGISTER_LSR);
            if (lsr.bits.thre == 1)
                break;
        }
        io_set_reg(io_base, UART_REGISTER_THR, uart_early_buf[get_idx]);
        get_idx++;
        get_idx %= BUF_SIZE;
    }

    mutex_release(&uart_lock);
}

void platform_dputc(char c)
{
    /* only print the log by uart for boot stage */
    if (!having_print_cb) {
        if (c == '\n')
            uart_putc('\r');
        uart_putc(c);
    }
}

int platform_dgetc(char *c, bool wait)
{
    return 0;
}

#if PRINT_USE_MMIO
void init_uart(void)
{
    status_t ret;
    uint64_t io_base = 0;

    io_base = (uint64_t)(pci_read32(0, 24, 2, 0x10) & ~0xF);
    ret = vmm_alloc_physical(vmm_get_kernel_aspace(), "uart", 4096,
        &mmio_base_addr, PAGE_SIZE_SHIFT, io_base, 0, ARCH_MMU_FLAG_UNCACHED_DEVICE);

    if (ret) {
        dprintf(CRITICAL, "%s: failed %d\n", __func__, ret);
        return;
    }

    uart_putc('\n');
}
#endif

