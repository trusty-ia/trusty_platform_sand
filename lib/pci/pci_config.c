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
#include <stdint.h>
#include <platform/sand_defs.h>
#include <platform/pci_config.h>

uint8_t hw_read_port_8(uint16_t port)
{
    uint8_t val8;

    __asm__ __volatile__ (
        "in %1, %0"
        : "=a" (val8)
        : "d" (port)
    );

    return val8;
}

uint16_t hw_read_port_16(uint16_t port)
{
    uint16_t val16;

    __asm__ __volatile__ (
        "in %1, %0"
        : "=a" (val16)
        : "d" (port)
    );

    return val16;
}

uint32_t hw_read_port_32(uint16_t port)
{
    uint32_t val32;

    __asm__ __volatile__ (
        "in %1, %0"
        : "=a" (val32)
        : "d" (port)
    );

    return val32;
}

void hw_write_port_8(uint16_t port, uint8_t val8)
{
    __asm__ __volatile__ (
        "out %1, %0"
        :
        : "d" (port), "a" (val8)
    );
}

void hw_write_port_16(uint16_t port, uint16_t val16)
{
    __asm__ __volatile__ (
        "out %1, %0"
        :
        : "d" (port), "a" (val16)
    );
}

void hw_write_port_32(uint16_t port, uint32_t val32)
{
    __asm__ __volatile__ (
        "out %1, %0"
        :
        : "d" (port), "a" (val32)
    );
}

uint8_t pci_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg)
{
    pci_config_address_t addr;

    addr.uint32 = 0;
    addr.bits.bus = bus;
    addr.bits.device = device;
    addr.bits.function = function;
    addr.bits.reg = reg;
    addr.bits.enable = 1;

    hw_write_port_32(PCI_CONFIG_ADDRESS_REGISTER, addr.uint32 & ~0x3);
    return hw_read_port_8(PCI_CONFIG_DATA_REGISTER | (addr.uint32 & 0x3));
}

void pci_write8(uint8_t bus,
            uint8_t device,
            uint8_t function,
            uint8_t reg,
            uint8_t value)
{
    pci_config_address_t addr;

    addr.uint32 = 0;
    addr.bits.bus = bus;
    addr.bits.device = device;
    addr.bits.function = function;
    addr.bits.reg = reg;
    addr.bits.enable = 1;

    hw_write_port_32(PCI_CONFIG_ADDRESS_REGISTER, addr.uint32 & ~0x3);
    hw_write_port_8(PCI_CONFIG_DATA_REGISTER | (addr.uint32 & 0x3), value);
}

uint16_t pci_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg)
{
    pci_config_address_t addr;

    addr.uint32 = 0;
    addr.bits.bus = bus;
    addr.bits.device = device;
    addr.bits.function = function;
    addr.bits.reg = reg;
    addr.bits.enable = 1;

    hw_write_port_32(PCI_CONFIG_ADDRESS_REGISTER, addr.uint32 & ~0x3);
    return hw_read_port_16(PCI_CONFIG_DATA_REGISTER | (addr.uint32 & 0x3));
}

void pci_write16(uint8_t bus,
            uint8_t device,
            uint8_t function,
            uint8_t reg,
            uint16_t value)
{
    pci_config_address_t addr;

    addr.uint32 = 0;
    addr.bits.bus = bus;
    addr.bits.device = device;
    addr.bits.function = function;
    addr.bits.reg = reg;
    addr.bits.enable = 1;

    hw_write_port_32(PCI_CONFIG_ADDRESS_REGISTER, addr.uint32 & ~0x3);
    hw_write_port_16(PCI_CONFIG_DATA_REGISTER | (addr.uint32 & 0x2), value);
}

uint32_t pci_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t reg)
{
    pci_config_address_t addr;

    addr.uint32 = 0;
    addr.bits.bus = bus;
    addr.bits.device = device;
    addr.bits.function = function;
    addr.bits.reg = reg;
    addr.bits.enable = 1;

    hw_write_port_32(PCI_CONFIG_ADDRESS_REGISTER, addr.uint32 & ~0x3);
    return hw_read_port_32(PCI_CONFIG_DATA_REGISTER);
}

void pci_write32(uint8_t bus,
            uint8_t device,
            uint8_t function,
            uint8_t reg,
            uint32_t value)
{
    pci_config_address_t addr;

    addr.uint32 = 0;
    addr.bits.bus = bus;
    addr.bits.device = device;
    addr.bits.function = function;
    addr.bits.reg = reg;
    addr.bits.enable = 1;

    hw_write_port_32(PCI_CONFIG_ADDRESS_REGISTER, addr.uint32 & ~0x3);
    hw_write_port_32(PCI_CONFIG_DATA_REGISTER, value);
}

uint64_t pci_read_bar0(uint16_t pci_addr)
{
    uint32_t low, high;
    uint8_t bus = GET_PCI_BUS(pci_addr);
    uint8_t dev = GET_PCI_DEVICE(pci_addr);
    uint8_t fun = GET_PCI_FUNCTION(pci_addr);

    low = pci_read32(bus, dev, fun, PCI_CONFIG_BAR_OFFSET);
    if ((low & 0x6) == 0x4) /* bit[2:1]: 00 for 32bit, 10 for 64bit, 01 and 11 are reserved */
        high = pci_read32(bus, dev, fun, PCI_CONFIG_BAR_OFFSET + 0x4);
    else
        high = 0;
    return MAKE64(high, low);
}
