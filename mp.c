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
#include <arch/mp.h>
#include <assert.h>
#include <trace.h>
#include <err.h>
#include <platform/interrupts.h>
#include <arch/ops.h>
#include <platform/sand.h>
#include <platform/vmcall.h>

status_t arch_mp_send_ipi(mp_cpu_mask_t target, mp_ipi_t ipi)
{
    make_schedule_vmcall();

    return NO_ERROR;
}

enum handler_return x86_ipi_generic_handler(void *arg)
{

    return INT_NO_RESCHEDULE;
}

enum handler_return x86_ipi_reschedule_handler(void *arg)
{
    return INT_RESCHEDULE;
}

void arch_mp_init_percpu(void)
{
    register_int_handler(INT_RESCH, &x86_ipi_reschedule_handler, NULL);
}
