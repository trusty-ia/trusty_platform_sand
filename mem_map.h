/*
 * Copyright (c) 2015 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* MUST keep consistent with macros defined in vendor/intel/fw/evmm */
#define STARTAP_IMG_SIZE    0x20000
#define SG_RUNTIME_SIZE     0xA00000

#define TARGET_MAX_MEM_SIZE 0x80000000

/* Value inherits RUNTIME_MEM_BASE from env. If not set, local value is used */
#ifndef RT_MEM_BASE
#define RT_MEM_BASE         0x12C00000
#endif

/* LK code entry offset to the LK memory start */
/* Must be larger than 12K and less than SG_RUNTIME_SIZE */
/* The memory is reserved for GDT, stack and MBI with 1 page each */
#define TRUSTY_ENTRY_OFFSET 0x3000


#define TRUSTY_START_ADDR   (RT_MEM_BASE + STARTAP_IMG_SIZE)
#define TRUSTY_ENTRY_ADDR   (TRUSTY_START_ADDR + TRUSTY_ENTRY_OFFSET)
#define TRUSTY_SIZE         SG_RUNTIME_SIZE
