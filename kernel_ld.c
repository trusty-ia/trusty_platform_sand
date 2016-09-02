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

#include "mem_map.h"

ENTRY(_start)
    SECTIONS
{
    .text TRUSTY_ENTRY_OFFSET : {
        __code_start = .;
        KEEP(*(.text.boot))
        *(.text* .sram.text)
        *(.gnu.linkonce.t.*)
        __code_end = .;
    } =0x9090
    .rela.dyn : {
        *(.rela.init)
        *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*)
        *(.rela.fini)
        *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*)
        *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*)
        *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*)
        *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*)
        *(.rela.ctors)
        *(.rela.dtors)
        *(.rela.got)
        *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
        *(.rela.ldata .rela.ldata.* .rela.gnu.linkonce.l.*)
        *(.rela.lbss .rela.lbss.* .rela.gnu.linkonce.lb.*)
        *(.rela.lrodata .rela.lrodata.* .rela.gnu.linkonce.lr.*)
        *(.rela.ifunc)
    }
    .rela.plt : {
        *(.rela.plt)
        *(.rela.iplt)
    }
    .rodata : ALIGN(4096) {
        __rodata_start = .;
        *(.rodata*)
        *(.gnu.linkonce.r.*)
        INCLUDE "arch/shared_rodata_sections.ld"
        . = ALIGN(8);
        __rodata_end = .;
    }
    .data : ALIGN(4096) {
        __data_start = .;
        *(.data .data.* .gnu.linkonce.d.*)
        INCLUDE "arch/shared_data_sections.ld"
    }
    .stack : ALIGN(4096){
        __tss_start = .;
        __tss_end = . + (4096);
    }
    __ctor_list = .;
    .ctors : { KEEP(*(.ctors)) }
    __ctor_end = .;
    __dtor_list = .;
    .dtors : { KEEP(*(.dtors)) }
    __dtor_end = .;
    .stab : { *(.stab) }
    .stabst : { *(.stabstr) }
    __data_end = .;
    .bss : ALIGN(4096) {
        __bss_start = .;
        *(.bss*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    }
    . = ALIGN(4096);
    _end = .;
    _end_of_ram = . + (4*1024*1024);
    /DISCARD/ : { *(.comment .note .eh_frame) }
}
