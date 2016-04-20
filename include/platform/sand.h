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

/* NOTE: keep arch/x86/crt0.S in sync with these definitions */

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

void platform_init_interrupts(void);
void platform_init_timer(void);
void platform_init_uart(void);

#endif
