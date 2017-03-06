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
#include <platform/sand.h>
#include <platform/vmcall.h>

#define TRUSTY_VMCALL_SMC 0x74727500 /* "tru" is 0x747275 */
#define TRUSTY_VMCALL_RESCHEDULE 0x74727501 /* "tru" is 0x747275 */

void make_smc_vmcall(smc32_args_t *args, long ret)
{
    __asm__ __volatile__(
		"pushq %%rbx;" /* save the ebx */
		"vmcall;"
		"mov %%ebx, %3;"
		"popq %%rbx;" /* restore the old ebx */

            :"=D"(args->smc_nr), "=S"(args->params[0]),
            "=d"(args->params[1]), "=r"(args->params[2])
            :"a"(TRUSTY_VMCALL_SMC), "D"(ret)
            );
}

void make_vmcall(uint32_t vmcall_id)
{
    __asm__ __volatile__ ("vmcall"::"a"(vmcall_id));
}

void make_schedule_vmcall()
{
    make_vmcall(TRUSTY_VMCALL_RESCHEDULE);
}
