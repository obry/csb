#include "iostream"
#include "ctime"
#include "uart.h"



//bool uart::init (const char * device, speed_t brate, int quiet)
uart::uart(const char * device, speed_t brate, int quiet)
{
  struct termios options;

  bytes_sent = 0;
  bytes_received = 0;
  if (quiet == 0) printf("Opening the device %s\n", device);
  debug=!quiet;

//  if (UART_fd != 0)
//    return true;

  /* open the serial device */
  if ((UART_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
    perror(device);
  }

  /* restore normal (blocking) behavior */
  fcntl(UART_fd, F_SETFL, 0);

  /* get the current options for the port... */
  tcgetattr(UART_fd, &options);

  /* see /usr/include/sys/termios.h */

/*
        B1200
        B1800
        B2400
        B4800
        B9600
        B19200
        B38400
        B57600
        B115200
        B128000
        B230400
        B256000
        B460800
        B500000
        B576000
        B921600
        B1000000
        B1152000
        B1500000
        B2000000
        B2500000
        B3000000
*/

  /* set the baud rates */
  cfsetispeed(&options, brate);
//  cfsetospeed(&options, brate);

  /* enable the receiver and set local mode... */
  options.c_cflag |= (CLOCAL | CREAD);

  /* set parity checking... */
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;

  /* set the character size... */
  options.c_cflag &= ~CSIZE; /* mask the character size bits */
  options.c_cflag |= CS8;    /* select 8 data bits */

  /* disable hardware flow control... */
  options.c_cflag &= ~CRTSCTS;

  /* disable software flow control... */
  options.c_iflag &= ~(IXON | IXOFF | IXANY);

  /* choose raw input... */
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  options.c_iflag &= ~(INLCR | ICRNL);

  /* choose raw output... */
  options.c_oflag &= ~OPOST;

  /* set timeout... */
  options.c_cc[VMIN] = 0;
  //  options.c_cc[VTIME] = 25; /* tenths of seconds */
  options.c_cc[VTIME] = 1; /* tenths of seconds */

  /* set the new options for the port... */
  tcsetattr(UART_fd, TCSANOW, &options);
}


//void uart::Exit ()
uart::~uart()
{
  /* close serial device */
  if (UART_fd != 0) {
    if (close(UART_fd) < 0) {
      perror(NULL);
    }
    UART_fd = 0;
  }
}

void uart::SendBreak (int duration)
{
    tcsendbreak(UART_fd, duration);
}


bool uart::WriteByte (uint8_t AByte)
{
    bytes_sent++;
    if (debug) printf("writebyte 0x%02x\n",AByte);
    if ( write(UART_fd, & AByte, 1 ) != 1 ) return false;
    return true;
}

bool uart::WriteBuffer (uint8_t * PByte, uint32_t nbytes)
{
    uint16_t i;
    if (debug)
        for (i=0; i<nbytes; i++)
            printf("writeburst 0x%02x\n",PByte[i]);
    bytes_sent+= write(UART_fd, PByte, nbytes );
    return true;
}


int uart::ReadByte ()
{
    uint8_t AByte;

    bytes_received++;
#ifdef EMULATION
return EOF;
#else
    if ( read( UART_fd, & AByte, 1 ) != 1 )
    {
//      printf("EOF when ReadByte!\n");
      return EOF;
    }

//    printf("ReadByte returns 0x%02x\n", AByte);
    return AByte;
#endif
}

int uart::ReadBuffer(uint8_t * PByte, uint32_t nbytes)
{
//    uint8_t AByte;
    uint32_t read_now, rest2read, r_now;

    rest2read = nbytes;
    while (rest2read > 0)
    {
//        printf("rest2read = 0x%04x, ", rest2read);
        if (rest2read > MAX2READ)
        {
       //     usleep(10000);
            read_now = MAX2READ;
        }
        else
        {
            read_now = rest2read;
        }
        rest2read -= read_now;
//        printf("-> read_now = %d, rest2read = 0x%04x \n", read_now, rest2read);

        r_now = read( UART_fd, PByte, read_now );
        if (r_now  != read_now )
        {
          printf("ReadBuffer : r_now is %d, requested %d\n", r_now, read_now);
          return EOF;
        }
        PByte += read_now;
    }

    bytes_received += nbytes;
    return 0;
}

/* END OF uart CLASS */

