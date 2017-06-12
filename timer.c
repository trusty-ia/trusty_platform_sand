/*******************************************************************************
 * Copyright (c) 2017 Intel Corporation
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
#include <err.h>
#include <platform.h>
#include <platform/interrupts.h>
#include <platform/timer.h>
#include <platform/sand.h>
#include <arch/ops.h>
#include <arch/x86.h>
#include <kernel/vm.h>
#include <platform/vmcall.h>

static volatile uint64_t timer_current_time; /* in ms */
uint64_t timer_delta_time; /* in ms */
volatile uint32_t trigger_pending_intr_50 = 0;

static platform_timer_callback t_callback;
static void *callback_arg;

#define TRUSTY_VMCALL_TIMER 0x74727503 /* "tru" is 0x747275 */
typedef enum {
    TIMER_MODE_PERIOD,
    TIMER_MODE_ONESHOT,
    TIMER_MODE_STOPPED,
} vmx_timer_mode_t;

static inline void make_timer_vmcall(vmx_timer_mode_t mode, uint64_t interval_in_ms)
{
    __asm__ __volatile__(
            "vmcall"
            ::"a"(TRUSTY_VMCALL_TIMER), "b"(mode), "c"(interval_in_ms)
            );
}

lk_time_t current_time(void)
{
    uint64_t val;
    uint32_t low, high;

    rdtsc(low, high);
    val = (uint64_t)((uint64_t)high <<32 | (uint64_t)low);
    timer_current_time = val/(uint64_t)(g_trusty_startup_info.calibrate_tsc_per_ms);

    return timer_current_time;
}

lk_bigtime_t current_time_hires(void)
{
    return timer_current_time * 1000;
}

#if PLATFORM_HAS_DYNAMIC_TIMER
void platform_trigger_soft_timer_intr(void)
{
    trigger_pending_intr_50 = 1;
    make_set_pending_intr_self_vmcall(INT_DYNC_TIMER);

    return;
}

status_t platform_set_oneshot_timer(platform_timer_callback callback,
        void *arg, lk_time_t interval)
{
    t_callback = callback;
    callback_arg = arg;
    timer_delta_time = interval;

    if(0 != is_lk_boot_complete) {
        platform_trigger_soft_timer_intr();
    }

    /* vmcall to set oneshot timer with interval (in ms) */
    //make_timer_vmcall(TIMER_MODE_ONESHOT, interval);

    return NO_ERROR;
}

void platform_stop_timer(void)
{
    if(0 != is_lk_boot_complete) {
        timer_delta_time = TRUSTY_STOP_TIMER;

        platform_trigger_soft_timer_intr();
    }
    return;
}

#else
status_t platform_set_periodic_timer(platform_timer_callback callback,
        void *arg, lk_time_t interval)
{
    t_callback = callback;
    callback_arg = arg;
    timer_delta_time = interval;

    return NO_ERROR;
}
#endif

static enum handler_return os_timer_tick(void *arg)
{
    if (!t_callback)
        return 0;

#ifndef PLATFORM_HAS_DYNAMIC_TIMER
    timer_current_time += timer_delta_time;
#endif
    lk_time_t time = current_time();
    return t_callback(callback_arg, time);
}

void platform_init_timer(void)
{
#ifndef PLATFORM_HAS_DYNAMIC_TIMER
    timer_current_time = 0;
#else
    lk_time_t time = current_time();

    timer_current_time = time;
#endif

    register_int_handler(INT_PIT, &os_timer_tick, NULL);
    unmask_interrupt(INT_PIT);
}

