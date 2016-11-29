/*******************************************************************************
 * Copyright (c) 2016 Intel Corporation
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
#ifndef __SAND_UART_H__
#define __SAND_UART_H__

#define UART_REGISTER_THR 0     /* WO Transmit Holding Register */
#define UART_REGISTER_RBR 0     /* RO Receive Buffer Register */
#define UART_REGISTER_DLL 0     /* R/W Divisor Latch LSB */
#define UART_REGISTER_DLM 1     /* R/W Divisor Latch MSB */
#define UART_REGISTER_IER 1     /* R/W Interrupt Enable Register */
#define UART_REGISTER_IIR 2     /* RO Interrupt Identification Register */
#define UART_REGISTER_FCR 2     /* WO FIFO Cotrol Register */
#define UART_REGISTER_LCR 3     /* R/W Line Control Register */
#define UART_REGISTER_MCR 4     /* R/W Modem Control Register */
#define UART_REGISTER_LSR 5     /* R/W Line Status Register */
#define UART_REGISTER_MSR 6     /* R/W Modem Status Register */
#define UART_REGISTER_SCR 7     /* R/W Scratch Pad Register */

typedef union {
    struct {
        uint8_t dr:1;
        uint8_t oe:1;
        uint8_t pe:1;
        uint8_t fe:1;
        uint8_t bi:1;
        uint8_t thre:1;
        uint8_t temt:1;
        uint8_t fifoe:1;
    } bits;
    uint8_t data;
} uart_lsr_t;

#endif
