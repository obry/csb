#ifndef CRC8_H
#define CRC8_H
// $Id: uart_classes.h 41 2015-04-22 13:09:52Z angelov $:

#include <stdio.h>   /* Standard input/output definitions, for perror() */
//#include <cstdint>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions, for close() */
#include <fcntl.h>   /* File control definitions, for open() */
#include <termios.h> /* POSIX terminal control definitions */
#include <signal.h>  /* ANSI C signal handling */
//#include <time.h>

#include <sys/time.h>


/* For cygwin */
/* For linux */
#define DEVICE_MASTER "/dev/ttyUSB0"

class crc8 {
  uint8_t   crc_reg;
public:
    // inits the register with 0xFF
    crc8();
    ~crc8();
    void init(void);
    // add a byte to the checksum
    void      addbyte(uint8_t AByte);
    // get the accumulated checksum
    uint8_t   get_crc(void);
};

#endif /* CRC8_H */

