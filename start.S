/*
 * Copyright (c) 2009 Corey Tabaka
 * Copyright (c) 2016 Travis Geiselbrecht
 * Copyright (c) 2017 Intel Corporation
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
#include <asm.h>
#include <arch/x86/descriptor.h>
#include <arch/x86/mmu.h>

#define MSR_EFER    0xc0000080
#define EFER_LME    0x00000100
#define MSR_PAT     0x277
#define CACHE_MODE  0x70106
#define MSR_GS_BASE 0xC0000101

#define PHYS_LOAD_ADDRESS (MEMBASE + KERNEL_LOAD_OFFSET)
#define PHYS_ADDR_DELTA (KERNEL_BASE + KERNEL_LOAD_OFFSET - PHYS_LOAD_ADDRESS)
#define PHYS(x) ((x) - PHYS_ADDR_DELTA)

#define PGDIR_SHIFT      39
#define PUD_SHIFT        30
#define PMD_SHIFT        21
#define PTD_SHIFT        12
#define PTRS_MASK        (512 - 1)

.section ".text.boot"

.align 8
.code64

/* Map 0 (VA) to 0 (PA), with 512Gb size. */
.macro map_low_512G
    /* Map the early bootstrap, 1G leaf */
    leaq pml4(%rip), %rdi
    leaq pdp(%rip), %rsi
    orq  $X86_KERNEL_PD_FLAGS, %rsi
    movq %rsi, (%rdi)

    leaq pdp(%rip), %rdi
    movq $512, %rcx
    xorq %rax, %rax

0:
    movq %rax, %rbx
    shlq $30, %rbx
    orq  $X86_KERNEL_PD_LP_FLAGS, %rbx
    movq %rbx, (%rdi)
    addq $8, %rdi
    incq %rax
    loop 0b
.endm

/* Map KERNEL_ASPACE_BASE (VA) to 0 (PA), with 512Gb size. */
.macro map_kernel_aspace
    /* Point the pml4e at the last 512G (kernel aspace 512GB mapping) */
    leaq PHYS(pdp_high)(%rbp), %rax
    leaq PHYS(pml4 + 8*511)(%rbp), %rdx
    orq  $X86_KERNEL_PD_FLAGS, %rax
    movq %rax, (%rdx)

    /* Set up a linear map of the first 512GB from 0xffffff8000000000 */
    leaq PHYS(linear_map_pdp_512)(%rbp), %rsi
    movq $0x40000, %rcx
    xorq %rax, %rax

    /* Loop across these page tables, incrementing the address by 2MB */
0:
    movq %rax, %rbx
    shl  $21, %rbx
    orq  $X86_KERNEL_PD_LP_FLAGS, %rbx    # lower word of the entry
    movq %rbx, (%rsi)

    addq $8,%rsi
    inc  %rax
    loop 0b

    /* Point the high pdp at our linear mapping page tables */
    leaq PHYS(pdp_high)(%rbp), %rsi
    movq $512, %rcx
    leaq PHYS(linear_map_pdp_512)(%rbp), %rax
    orq  $X86_KERNEL_PD_FLAGS, %rax

0:
    movq %rax, (%rsi)
    addq $8, %rsi
    addq $4096, %rax
    loop 0b
.endm

/*
 * Map KERNEL_BASE (VA) to actual runtime address (PA), with 16MB size.
 * We are using 0 MEMBASE and KERNEL_LOAD_OFFSET, if you change either
 * of these settings, please update offset of pde_kernel which would be
 * filled with pte_kernel to adapt your changes.
 */
.macro  map_kernel_base_region
    /* Max memory -2G adress of VA */
    leaq pdp_high + 8*510(%rip), %rdi
    leaq pde_kernel(%rip), %rsi
    orq  $X86_KERNEL_PD_FLAGS, %rsi
    movq %rsi, (%rdi)

    /* Map 8*2M=16M, this is hard code */
    leaq pde_kernel(%rip), %rdi
    leaq pte_kernel(%rip), %rsi
    movq $8, %rcx
0:
    orq  $X86_KERNEL_PD_FLAGS, %rsi
    movq %rsi, (%rdi)
    addq $8, %rdi
    addq $4096, %rsi
    loop 0b

/*
 * Start to fill pte with physical address.
 * RBP contains actual physcial address where Trusty binary resides.
 */
    movq %rbp, %rsi
    leaq pte_kernel(%rip), %rdi
    movq $4096, %rcx
0:
    orq  $103, %rsi
    movq %rsi, (%rdi)
    addq $8, %rdi
    addq $4096, %rsi
    loop 0b
.endm

.global _start
_start:
    call 1f
1:
    pop %rbp
    sub $PHYS(1b), %rbp

    /* Zero the bss section */
    lea __bss_start(%rip), %rax
    lea __bss_end(%rip), %rcx
    sub %rax, %rcx
    shr $2, %rcx
2:
    movq $0, (%rax)
    add $4, %rax
    loop 2b

    /* Map 0 ~ 512G */
    map_low_512G

    /* Map for kernel address space */
    map_kernel_aspace

    /* Map for kernel symbols */
    map_kernel_base_region

    /* Save the run addr */
    leaq _start(%rip), %rax
    movq %rax, entry_phys(%rip)

    leaq pml4(%rip), %rax
    movq %rax, %cr3
    /* Load our gdtr */
    lgdt _gdtr

    /* Set PAT MSR */
    movl $MSR_PAT, %ecx

    movl $CACHE_MODE, %eax
    movl $CACHE_MODE, %edx
    wrmsr

    /* Long jump to our code selector and the high address */
    push  $CODE_64_SELECTOR
    push  $highaddr
    lretq

highaddr:
    xorq %rax, %rax
    mov  %ax,  %gs

    /* Set TR now, since lk_main check cpuid when initialing thread */
    mov  $TSS_SELECTOR, %ax
    ltr  %ax

    /* Load the high kernel stack */
    mov  $(_kstack + 4096), %rsp

    /* Reload the gdtr */
    lgdt _gdtr

#ifdef STACK_PROTECTOR
    /* Setup stack check guard for C call */
    leaq global_states + 0x28(%rip), %rdi
    call get_rand_64
    subq $0, %rax
    jz   0f
#endif

    /* Set up the idt */
    call setup_idt

    /* Set up GS base */
    leaq global_states(%rip), %rax

    movq %rax, %rdx
    shr  $32,  %rdx
    movq $MSR_GS_BASE, %rcx
    wrmsr

    xorq %rbp, %rbp
    xorq %rdi, %rdi
    xorq %rsi, %rsi
    xorq %rdx, %rdx
    xorq %rcx, %rcx

    /* Call the main module */
    call lk_main

0:                          /* just sit around waiting for interrupts */
    hlt                     /* interrupts will unhalt the processor */
    pause
    jmp 0b                  /* so jump back to halt to conserve power */

/* Space for 4GB temporary page tables */
.balign 4096
pml4_tmp:
    .fill 4096, 1, 0
pdpt_tmp:
    .fill 4096, 1, 0
pd_tmp:
    .fill 4*4096, 1, 0

.global _start_pa
.set _start_pa, _start - KERNEL_BASE
