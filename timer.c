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
#include <platform.h>
#include <platform/interrupts.h>
#include <platform/timer.h>
#include <platform/sand.h>

static volatile uint64_t timer_current_time; /* in ms */
static uint64_t timer_delta_time; /* in ms */

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
    return timer_current_time;
}

lk_bigtime_t current_time_hires(void)
{
    return timer_current_time * 1000;
}

#if PLATFORM_HAS_DYNAMIC_TIMER
status_t platform_set_oneshot_timer(platform_timer_callback callback,
        void *arg, lk_time_t interval)
{
    t_callback = callback;
    callback_arg = arg;
    timer_delta_time = interval;

    /* vmcall to set oneshot timer with interval (in ms) */
    make_timer_vmcall(TIMER_MODE_ONESHOT, interval);
}

void platform_stop_timer(void)
{
    /* vmcall to stop timer */
    make_timer_vmcall(TIMER_MODE_STOPPED, 0);
}
#else
status_t platform_set_periodic_timer(platform_timer_callback callback,
        void *arg, lk_time_t interval)
{
    t_callback = callback;
    callback_arg = arg;
    timer_delta_time = interval;

    /* vmcall to set period timer with interval (in ms) */
#if 0/*when using proxied timer disable periodic timer*/
    make_timer_vmcall(TIMER_MODE_PERIOD, interval);
#endif
}
#endif

static enum handler_return os_timer_tick(void *arg)
{
    timer_current_time += timer_delta_time;
    lk_time_t time = current_time();
    return t_callback(callback_arg, time);
}

void platform_init_timer(void)
{
    timer_current_time = 0;
    register_int_handler(INT_PIT, &os_timer_tick, NULL);
    unmask_interrupt(INT_PIT);
}

