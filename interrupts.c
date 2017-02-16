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
#include <sys/types.h>
#include <debug.h>
#include <err.h>
#include <reg.h>
#include <kernel/thread.h>
#include <platform/interrupts.h>
#include <arch/ops.h>
#include <arch/x86.h>
#ifdef ENABLE_FPU
#include <arch/fpu.h>
#endif
#include <kernel/spinlock.h>
#include <platform/sand.h>
#include <lib/sm.h>

static spin_lock_t lock;

void x86_gpf_handler(struct x86_iframe *frame);
void x86_invop_handler(struct x86_iframe *frame);
void x86_unhandled_exception(struct x86_iframe *frame);
#ifdef ARCH_X86_64
void x86_pfe_handler(struct x86_iframe *frame);
#endif

struct int_handler_struct {
    int_handler handler;
    void *arg;
};

static struct int_handler_struct int_handler_table[INT_VECTORS];

void issueEOI(unsigned int vector)
{
}

void platform_init_interrupts(void)
{
    /*
     * Mask all interrputs before LK bootup
     */
    x86_set_cr8(0xF);
}

status_t mask_interrupt(unsigned int vector)
{
    if (vector >= INT_VECTORS)
        return ERR_INVALID_ARGS;

    return NO_ERROR;
}

void platform_mask_irqs(void)
{
}

status_t unmask_interrupt(unsigned int vector)
{
    if (vector >= INT_VECTORS)
        return ERR_INVALID_ARGS;

    return NO_ERROR;
}

extern enum handler_return sm_handle_irq(void);

#define TRUSTY_VMCALL_PENDING_INTR 0x74727505 /* "tru" is 0x747275 */

static inline void set_pending_intr_to_ns(uint8_t vector)
{
    if (vector > INT_VECTORS) {
        dprintf(CRITICAL, "error: internal failure in LK\n");
        return;
    }

    __asm__ __volatile__(
            "pushq %%rbx \n\t"
            "mov %%rcx, %%rbx \n\t"
            "vmcall \n\t"
            "popq %%rbx \n\t"
            ::"a"(TRUSTY_VMCALL_PENDING_INTR), "c"(vector)
            );
}

enum handler_return platform_irq(struct x86_iframe *frame)
{
    /* get the current vector */
    unsigned int vector = frame->vector;

    THREAD_STATS_INC(interrupts);

    /* deliver the interrupt */
    enum handler_return ret = INT_NO_RESCHEDULE;

    switch (vector) {
        case INT_GP_FAULT:
            x86_gpf_handler(frame);
            break;

        case INT_INVALID_OP:
            x86_invop_handler(frame);
            break;
        case INT_PAGE_FAULT:
#ifdef ARCH_X86_64
            x86_pfe_handler(frame);
#endif
            break;

        case INT_DEV_NA_EX:
#ifdef ENABLE_FPU
            fpu_dev_na_handler();
            break;
#endif
        case INT_MF:
        case INT_XM:
        case INT_DIVIDE_0:
        case INT_DEBUG_EX:
        case INT_STACK_FAULT:
        case 3:
            x86_unhandled_exception(frame);
            break;

#if ENABLE_TRUSTY_SIMICS
            /* Workaround for Simics modeling issue. TODO: remove this when GSD tape in. */
#define INT_RESERVED_15 0xf
        case INT_RESERVED_15:
            break;
#endif

        case INT_DYNC_TIMER:
            if (!trigger_soft_intr_50) {
                set_pending_intr_to_ns(vector);
                ret = sm_handle_irq();
            } else {
                sm_handle_irq();
            }
            break;

        default:
            if (int_handler_table[vector].handler) {
                ret = int_handler_table[vector].
                    handler(int_handler_table[vector].arg);
            } else {
                set_pending_intr_to_ns(vector);
                /*
                 * CAUTION: smc to non-secure world, and will not
                 * return unless Android call smc to secure world
                 */
                ret = sm_handle_irq();
            }
    }

    /*
     * Ack the interrupt
     * Please issue EOI in interrupt handler
     */
    //issueEOI(vector);

    return ret;
}

void register_int_handler(unsigned int vector, int_handler handler, void *arg)
{
    if (vector >= INT_VECTORS)
        panic("register_int_handler: vector out of range %d\n", vector);

    spin_lock_saved_state_t state;
    spin_lock_irqsave(&lock, state);

    int_handler_table[vector].arg = arg;
    int_handler_table[vector].handler = handler;

    spin_unlock_irqrestore(&lock, state);
}

/*
 * Stubs to resolve compilations errors.  These and other
 * such functions implemented under flags like SM_LIB need
 * to be implemented
 */
status_t sm_intc_fiq_enter(void)
{
    return 0;
}

void sm_intc_fiq_exit(void)
{
}

long smc_intc_get_next_irq(smc32_args_t *args)
{
    long vector;

    for (vector = args->params[0]; vector<INT_VECTORS; vector++)
    {
        if (int_handler_table[vector].handler)
            return vector;
    }
    return -1;
}

long smc_intc_request_fiq(smc32_args_t *args)
{
    return NO_ERROR;
}

