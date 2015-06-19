#ifndef UART_CRC_H
#define UART_CRC_H
// $Id: uart_classes.h 41 2015-04-22 13:09:52Z angelov $:

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

#define MAX2READ 1


int32_t sign_ext24(uint32_t adc);


class uart
{
    int UART_fd;            // File descriptor for serial device
    uint8_t debug;
public:
    int bytes_sent;         // a counter for the bytes sent (with payload)
    int bytes_received;     // a counter for the bytes received (with payload)
    // quiet = 1 to suppress more messages
    uart(const char * device, speed_t brate, int quiet = 0);
    ~uart();
    // send a break in case of lost synchronization, duration = 1.
    void SendBreak (int duration);
    // read a Byte, returns EOF in case of error or the Byte
    int  ReadByte ();
    int ReadBuffer (uint8_t * PByte, uint32_t nbytes);
    // write a Byte, returns false in case of error, otherwise true
    bool WriteByte (uint8_t AByte);
    bool WriteBuffer (uint8_t * PByte, uint32_t nbytes);
};

#endif /* UART */
