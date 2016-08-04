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
#ifndef __SAND_H
#define __SAND_H

#include <stdint.h>

/* NOTE: keep arch/x86/crt0.S or crt0_64.S in sync with these definitions */

/* interrupts */
#define INT_VECTORS 0xFF

/* defined interrupts */
#define INT_PIT             0x31 /* 0x31 is not used in Android */
#define INT_RESCH           0x80

/* exceptions */
#define INT_DIVIDE_0        0x00
#define INT_DEBUG_EX        0x01
#define INT_INVALID_OP      0x06
#define INT_DEV_NA_EX       0x07
#define INT_MF              0x10
#define INT_XM              0x13

/* faults */
#define INT_STACK_FAULT     0x0c
#define INT_GP_FAULT        0x0d
#define INT_PAGE_FAULT      0x0e

#define KILOBYTE    *1024
#define KILOBYTES   KILOBYTE
#define MEGABYTE    *1024 KILOBYTES
#define MEGABYTES   MEGABYTE
#define GIGABYTE    *1024 MEGABYTES
#define GIGABYTES   GIGABYTE


/* Please keep align with definition in iKGT trusty_guest.h*/
typedef struct _trusty_startup_info{
    /* Used to double check structures match */
    uint32_t size_of_this_struct;

    /* Heap size in MB asigned to LK */
    uint32_t heap_size_in_mb;

    /* Used to calibrate TSC in LK */
    uint64_t calibrate_tsc_per_sec;

    /* Used by keymaster */
    uint64_t trusty_mem_base;
}trusty_startup_info_t;

extern trusty_startup_info_t* g_trusty_startup_info;

void platform_init_interrupts(void);
void platform_init_timer(void);
void platform_init_uart(void);
void clear_sensitive_data(void);

#endif
