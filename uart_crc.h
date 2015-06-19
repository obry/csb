//#ifndef UART_CRC_H
//#define UART_CRC_H

// $Id: uart_classes.h 41 2015-04-22 13:09:52Z angelov $:

#include "crc8.h"
#include "uart.h"

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


/* For cygwin */
/* For linux */
#define DEVICE_MASTER "/dev/ttyUSB0"

//#define pfsize0     2
//#define pfsize1     4
//#define pfsize2     6
//#define pfsize3     8
//#define pfsize4    16
//#define pfsize5   128
//#define pfsize6   512
//#define pfsize7  2048

//uint16_t bsize(uint8_t psize);

#define AUTOINC_ADDR        0x8000
#define NO_AUTOINC_ADDR     0x7FFF

class uart_crc
{
public:
    uart *uart_phys;        // a pointer to the uart physical layer (RS-232)
    uint8_t debug;
public:
    // initialize the pointer
    uart_crc(uart *uart_used);
    ~uart_crc();
    // slaveID = 0 to 14 for single module read/write or 15 for broadcast
    // psize = 1 to 8 is the number of the data bytes to be send/received
    // saddr = 0 to 255 is the internal start address
    // *Bytes is a pointer to an array of Bytes to be send/received
    // ignore_crc should be false, only when reading with broadcast the presence or alarm register
    // use true to avoid wrong error messages.

    // send a burst of 1 to 8 bytes
    bool SendBurst(uint8_t slaveID, uint16_t psize, uint16_t saddr, uint8_t *Bytes, bool ignore_crc);
    // read a burst of 1 to 8 bytes
    bool RecvBurst(uint8_t slaveID, uint16_t psize, uint16_t saddr, uint8_t *Bytes, bool ignore_crc);
    // read a burst of 2 bytes
    uint8_t  ReadByte(uint8_t slaveID, uint16_t saddr, bool ignore_crc);
    uint16_t ReadWord(uint8_t slaveID, uint16_t saddr, bool ignore_crc);
    uint32_t Read3Bytes(uint8_t slaveID, uint16_t saddr, bool ignore_crc);
    bool     WriteByte(uint8_t slaveID, uint16_t saddr, uint8_t wdata, bool ignore_crc);
    bool     WriteWord(uint8_t slaveID, uint16_t saddr, uint16_t wdata, bool ignore_crc);
};

//#endif /* UART_CRC */

