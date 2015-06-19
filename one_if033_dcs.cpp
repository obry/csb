#include "iostream"
#include "ctime"
#include "uart_crc.h"
#include "one_if033_dcs.h"

one_if033_dcs::one_if033_dcs(uint8_t new_debug, uart_crc *p_uart_crc, uint8_t new_slave_addr)
: one_if033(new_debug, p_uart_crc, new_slave_addr) // explicitly call the base class constructor. This avoids that the default constructor is called (which is not defined)
{
  m_uart_crc = p_uart_crc;
  debug = new_debug;
  slave_addr = new_slave_addr;
  last_status = 0xFF; // unknown
}

one_if033_dcs::~one_if033_dcs()
{
    // do nothing
}


