// $Id: Cone_if033.h 11 2014-12-03 12:38:53Z angelov $:

//#include "uart_classes.h"

#ifdef _WIN32
        #include <windows.h> // for the Sleep function
   #define sleep Sleep
   #define usleep(A) Sleep(A/1000)
#else
        #include <unistd.h>
        #define Sleep(A) usleep(A*1000)
#endif

class one_if033
{
private:
    uint8_t debug;
    uint32_t slave_addr;

    uart_crc *m_uart_crc;

public:
    one_if033(uint8_t new_debug, uart_crc *p_uart_crc, uint8_t new_slave_addr);
    ~one_if033();

    int32_t  set_dac(uint16_t dac);

};
