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
#include <platform/lkguest.h>

static uint8_t buff_is_overflow[] = "buffer is overflow!";

extern having_print_cb;

uint64_t g_mmio_base_addr = 0;

struct conbuff teecons_buff = {
    .cmd = 0,
    .status = TEE_DBG_BUFF_VALID,
    .len = 0};

void tc_flush(void)
{
#if LK_DEBUGLEVEL > 1
    lkguest_teewrite(&teecons_buff);

    memset(teecons_buff.msg, 0, TEE_DBG_BUFFERSIZE);
    teecons_buff.len = 0;
    teecons_buff.status = TEE_DBG_BUFF_VALID;
#endif
}

void tee_putc(int c)
{
    int i;
    struct conbuff *p = &teecons_buff;

    if(p->status == TEE_DBG_BUFF_INVALID)
        return;

    p->msg[p->len++] = c;

    if(c == '\r' || c == '\n')
        tc_flush();

    /* the dbg buffer is full, don't write anymore, fill with string:
     * "buffer is overflow!" at the end to mark the buffer is full.
     */
    if (p->len > TEE_DBG_BUFFERSIZE-sizeof(buff_is_overflow)-2) {
        for (i=0; i<sizeof(buff_is_overflow); i++)
            p->msg[p->len++] = buff_is_overflow[i];

        p->msg[p->len] = '\0';
        p->status = TEE_DBG_BUFF_INVALID;

        /*  call VMCALL to flush the log to vmm */
        tc_flush();
    }
}

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
    uint64_t io_base = 0;
    /*MMIO HSUART2: 00.24.2
     *bus:0; device:24; function:2; reg:0x10*/
    io_base = (uint64_t)(pci_read32(0, 24, 2, 0x10) & ~0xF);
    return io_base;
}

static void uart_putc(char c)
{
    uart_lsr_t lsr;

    if (!g_mmio_base_addr)
        g_mmio_base_addr = get_mmio_base();

    while (1) {
        lsr.data = uart_mmio_get_reg(g_mmio_base_addr, UART_REGISTER_LSR);
        if (lsr.bits.thre == 1)
            break;
    }
    uart_mmio_set_reg(g_mmio_base_addr, UART_REGISTER_THR, c);
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

