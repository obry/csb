#include "uart_crc.h"
#include "uart.h"
#include "one_if033.h"
#include "one_if033_dcs.h"


//uint8_t  cfg_data[MAX_CFG_DATA], ret;
uart     *my_uart_pl;
uart_crc *my_uart_crc;
//number of current sense boards:  
const uint16_t nif033=8;
one_if033_dcs *p_if033[nif033];  
//uint16_t idx, slv_id, presamples, dac, testpatt, trigg_ena, threshold, i2c_not_init, adc_dis;
uint16_t slv_id, idx;
uint8_t iff033_status[nif033];
uint8_t iff033_isFull[nif033];
int32_t iff033_triggTime[nif033];
uint32_t *time_1Hz;
uint32_t *time_10MHz;
//uint32_t br, pings, nbytes, i;
uint32_t br;
char dev_id[512];


