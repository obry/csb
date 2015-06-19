#ifndef ONE_IF033_DCS_H
#define ONE_IF033_DCS_H


#include <stdio.h>   /* Standard input/output definitions, for perror() */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions, for close() */
#include <fcntl.h>   /* File control definitions, for open() */
#include <termios.h> /* POSIX terminal control definitions */
#include <signal.h>  /* ANSI C signal handling */
//#include <time.h>

#include <sys/time.h>

#include "one_if033.h"

class one_if033_dcs : public one_if033 {
  private:
    uint8_t debug;
    uint8_t last_status;
    uint32_t slave_addr;

    uart_crc *m_uart_crc;
  public:
    one_if033_dcs(uint8_t new_debug, uart_crc *p_uart_crc, uint8_t new_slave_addr);
    ~one_if033_dcs();


};

#endif
