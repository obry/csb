// $Id: uart_test.cpp 42 2015-04-25 11:46:48Z mfleck $:

#include <stdio.h>   /* Standard input/output definitions, for perror() */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions, for close() and sleep(seconds) */
#include <fcntl.h>   /* File control definitions, for open() */
#include <termios.h> /* POSIX terminal control definitions */
#include <signal.h>  /* ANSI C signal handling */
#include <time.h>
#include <string>
#include <iostream>
#include <dis.hxx>

#include <sys/time.h>
//stty -F /dev/ttyUSB0 115200

#include "uart_crc.h"
#include "uart.h"
#include "one_if033.h"
#include "one_if033_dcs.h"

#define DEVICE_MASTER "/dev/ttyUSB0"
#define DEF_BRATE 2000000
#define DEF_SLVID 7

using namespace std;

// ####### FSM #######
typedef enum {
        ST_triggered,
        ST_ready
} FSM_STATE;

typedef enum {
        EVT_trig,
        EVT_goready
} FSM_EVENTS;

// ###################

int32_t read_hex_dec(char *c, uint32_t *res);

speed_t int2speed_t(uint32_t br)
{
  if (br==4000000) return B4000000;
  if (br==3000000) return B3000000;
  if (br==2500000) return B2500000;
  if (br==2000000) return B2000000;
  if (br==1500000) return B1500000;
  if (br==1152000) return B1152000;
  if (br==1000000) return B1000000;
  if (br== 921600) return  B921600;
  if (br== 576000) return  B576000;
  if (br== 500000) return  B500000;
  if (br== 460800) return  B460800;
  if (br== 230400) return  B230400;
  if (br== 115200) return  B115200;
  if (br==  57600) return   B57600;
  if (br==  38400) return   B38400;
  if (br==  19200) return   B19200;
  if (br==   9600) return    B9600;
  if (br==   4800) return    B4800;
  if (br==   2400) return    B2400;
  if (br==   1200) return    B1200;
  printf("unknown bitrate %d, using 2000000\n",br);
  return B2000000;
}

uint8_t gen_psrg(uint8_t present)
{
  uint8_t b0, b1, newbit;

  b0 = present >> 7;
  b1 = (present >> 6) & 1;
  newbit = b0 ^ b1;
  newbit &= 1;
  return (present << 1) | newbit;
}

void print_usage(void)
{
  printf("if033 [options]\n");
  printf("--dev <device_id>, like /dev/ttyUSB0, default %s\n", DEVICE_MASTER);
  printf("--br <1200|2400|4800...115200|...1000000|...|4000000>, default %d\n", DEF_BRATE);
  printf("--slv <1..126> : slave id, default %d\n", DEF_SLVID);
  printf("--pingc <pings> : send <pings> times a byte-counter to the UART\n");
  printf("--pinga <pings> : send <pings> times 0xAA 0x55 to the UART\n");
  printf("--pre <0..4095> : set the presamples\n");
  printf("--dac <0..4095> : set the dac\n");
  printf("--dacf <0..4095> : set the flash & dac register\n");
  printf("--dacramp : set the dac from 0 to 4095 with step 1\n");
  printf("--thresh <adc_dis> <tp> <trigg_ena> <threshold> : set test pattern (0|1) trigger enable (0|1) and threshold (0..4095)\n");
  printf("--trigg : send software trigger command\n");
  printf("--reset : send software reset command\n");
  printf("--bufclr : send buffer clear command\n");
  printf("--timerclr : send timer clear command\n");
  printf("--status : read and print the status\n");
  printf("--ucode <32-bit user code> : set the user code for subsequent programming of FPGA internal flash\n");
  printf("--er_cfg : erase the CFG flash\n");
  printf("--er_ufm : erase the UFM\n");
  printf("--rd_cfg <size> <filename> : read from CFG <size> bytes and store to <filename>, <size>=0 means auto, trailing 0x00 omittet\n");
  printf("--rd_ufm <size> <filename> : read from UFM <size> bytes and store to <filename>, <size>=0 means auto\n");
  printf("--wr_cfg <size> <filename> : write to CFG <size> read from <filename>, <size>=0 means auto\n");
  printf("--wr_ufm <size> <filename> : write to UFM <size> random bytes and store them to the <filename>, <size>=0 means auto\n");
  printf("-o <output file> : read the event buffer to <output file>\n");
}


//timestamp
string get_current_time()
{
  //returns the time upon calling this function
  char prg_id[50];
  time_t t = time(0);   // get time now
  struct tm * now = localtime( & t );
  //prg_id = ( now->tm_sec << 24) | ( now->tm_min  << 18) | ( now->tm_hour  << 13) | (now->tm_mday  << 8) | ((now->tm_mon+1)     << 4) | ((now->tm_year+1900 - 2015) & 0xF);
  sprintf(prg_id,"%4d-%02d-%02d at %02d:%02d:%02d", now->tm_year+1900,now->tm_mon+1,now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec); //   
  return prg_id;
}




#define debug_main 0

int main(int argc, char** argv)
{
  //uint8_t  cfg_data[MAX_CFG_DATA], ret;
  uart     *my_uart_pl;
  uart_crc *my_uart_crc;
  //number of current sense boards:  
  const uint16_t nif033=2;
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
  speed_t br_speed;
  //FILE *f_out, *f_inp;
  //char out_file[512];
  //char inp_file[512];
  char dev_id[512];
  //uint32_t ucode = 0;

  strcpy(dev_id, DEVICE_MASTER); // default
  br = DEF_BRATE;
  slv_id = DEF_SLVID;



  //i2c_not_init=1;


  //if (argc < 2)
  //{
  //  printf("program path: %s\n",argv[0]);
  //  print_usage();
  //  return(1);
  //}
  //    else
  //    for (idx=1; idx<argc; idx++)
  //    {
  //        if (debug_main)
  //            printf("Current cmd line option is %s\n",argv[idx]);
  //        if (strcmp(argv[idx],"--dev") == 0 )
  //        {
  //            idx++;
  //            strcpy(dev_id, argv[idx]);
  //        }
  //        else
  //        if (strcmp(argv[idx],"--br") == 0 )
  //        {
  //            idx++;
  //            br = atoi(argv[idx]);
  //        }
  //        else
  //        if (strcmp(argv[idx],"--slv") == 0 )
  //        {
  //            idx++;
  //            slv_id = atoi(argv[idx]);
  //        }
  //    }

  // ############# DIM stuff ############# //
  char triggerTimeStamp[nif033][50];
  DimService * ds_timeStamp[nif033];
  for(idx=0;idx<nif033;idx++)
  {
    char name[50];
    sprintf(name,"if033_%d",idx);
    ds_timeStamp[idx] = new DimService(name,triggerTimeStamp[idx]);
    //DimService runNumber("iff033_1",triggerTimeStamp[idx]); 
  }
  DimServer::setDnsNode("x200s");
  DimServer::start("trd_hv_spikes"); 
  // ##################################### //
  

  br_speed = int2speed_t(br);
  my_uart_pl = new uart(dev_id, br_speed, 1);
  my_uart_crc = new uart_crc(my_uart_pl);

  for(idx=0;idx<nif033;idx++)
  {
    p_if033[idx] = new one_if033_dcs(0, my_uart_crc, slv_id); // need for every iff033 an instance of one_if033 with individual slave id
  }
  printf("UART %s open at speed %d\n", dev_id, br);

  string st[nif033];

  while(1)
  {
    for(idx=0;idx<nif033;idx++)
    {
      iff033_status[idx] = p_if033[idx]->get_status(); //get_status() to update e.g. trigger condition of a if033 board
      iff033_isFull[idx] = p_if033[idx]->last_full();
      //printf("isFull = %d \n", isFull);
      if(iff033_isFull[idx] == 1)
      {
	//iff033_triggTime[idx] = p_if033[idx]->get_timer(time_1Hz,time_10MHz);
	//printf("if033_%d is full. Timestamp: %d \n", idx,iff033_triggTime[idx]);
	//printf("if033_%d is full. Timestamp: %d \n", idx,get_current_time());
	st[idx] = get_current_time();
	cout << st[idx] <<endl;
	strcpy(triggerTimeStamp[idx], "Triggered");
	cout << triggerTimeStamp[idx] <<endl;
    	ds_timeStamp[idx]->updateService();

	//reset of trigger needed here.
	p_if033[idx]->send_cmd(CMD_SOFT_RST);
      }
    }
    sleep(1);
  }
}

int32_t read_hex_dec(char *c, uint32_t *res)
{
  int32_t e,d;
  uint32_t w;
  e = -1;
  if(strlen(c) > 2)
  {
    if (strncmp(c, "0x",2) == 0) // hex
    {
      e = sscanf(c, "0x%x", &w);
      *res = w;
    } else { // dec
      e = sscanf(c, "%d", &d);
      *res = d;
    }
  }
  else
  { // short dec
    e = sscanf(c, "%d", &d);
    *res = d;
  }

  if (e == 1) return 0;
  else return -1;
}

