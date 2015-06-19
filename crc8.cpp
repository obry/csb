#include "iostream"
#include "ctime"
#include "crc8.h"

// $Id: uart_classes.cpp 41 2015-04-22 13:09:52Z angelov $:

//#define EMULATION


uint8_t crc8::get_crc(void)
{
    return crc_reg;
}

crc8::crc8()
{
    crc_reg = 0xFF;
}

crc8::~crc8()
{
}

void crc8::init(void)
{
    crc_reg = 0xFF;
}

void crc8::addbyte(uint8_t AByte)
{
    const uint8_t PolyD = 0x8C;
    uint8_t cbit, dbit, i;

    for (i = 0; i < 8; i++)
        {
            cbit = crc_reg & 1;
            dbit = AByte   & 1;
            crc_reg = crc_reg >> 1;
            if (cbit != dbit) crc_reg ^= PolyD;
            AByte = AByte >> 1;
        }
}

