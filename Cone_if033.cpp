// $Id: Cone_if033.cpp 11 2014-12-03 12:38:53Z angelov $:

#include "Cone_if033.h"

one_if033::one_if033(uint8_t new_debug, uart_crc *p_uart_crc, uint8_t new_slave_addr)
{
    m_uart_crc = p_uart_crc;
    debug = new_debug;
    slave_addr = new_slave_addr;
}

one_if033::~one_if033()
{
    // do nothing
}



int32_t one_if033::set_dac(uint16_t dac)
{
    return m_uart_crc->WriteWord(slave_addr, 8, dac, false);
}


