#ifndef ONE_IF033_H
#define ONE_IF033_H

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

#define ADDR_COMMAND        0x00
#define ADDR_STATUS         0x00
#define ADDR_EV_CNT         0x02
#define ADDR_THRESHOLD      0x04
#define ADDR_PRESAMPLES     0x06
#define ADDR_DAC            0x08
#define ADDR_TIMER_10MHZ    0x0A
#define ADDR_TIMER_1HZ      0x0D
#define ADDR_EV_BUFFER      0x0F
#define ADDR_SVN            0x10
#define ADDR_COMPILE        0x13
#define ADDR_DPRAM          0x2000
#define ADDR_IMEM           0x1000
#define IMEM_SIZE           512


#define ADDR_DIS            0x04


#define CMD_BUFF_CLR        (1 << 7)
#define CMD_TIMR_RST        (1 << 6)
#define CMD_TIMR_SYNC       (1 << 5)
#define CMD_SOFT_TRIG       (1 << 4)
#define CMD_SOFT_RST        (1 << 3)
#define CMD_CPU_RST         (1 << 4)

#define STATUS_FULL         (1 << 7)
#define STATUS_EMPTY        (1 << 6)
#define STATUS_EFLASH        1
#define ADC_SAMPLES         4096

#define DAC_SLV_ADDR        0xC0 // shifted left
#define ADC_SLV_ADDR        0x68 // shifted left

// EFB : second I2C master
#define I2C2_CR             0x4A
#define I2C2_CMDR           0x4B
#define I2C2_TXDR           0x4E
#define I2C2_BR0            0x4C
#define I2C2_BR1            0x4D
#define I2C2_SR             0x4F
#define I2C2_RXDR           0x51

// EFB : interface to flash memory
#define WB_CFGCR            0x70
#define WB_CFGTXDR          0x71
#define WB_CFGSR            0x72
#define WB_CFGRXDR          0x73
#define WB_CFGIRQ           0x74
#define WB_CFGIRQEN         0x75
#define WB_EFBIRQ           0x77

#define WB_MAX_BUSY         1000
#define MAX_CFG_DATA        500000

#define display_debug 0


class one_if033
{
private:
    uint8_t debug;
    uint8_t last_status;
    uint32_t slave_addr;

    uart_crc *m_uart_crc;

public:
    one_if033(uint8_t new_debug, uart_crc *p_uart_crc, uint8_t new_slave_addr);
    ~one_if033();
    // specific to IF033 - one channel
    int32_t  send_cmd(uint8_t cmd); // send commands, see the CMD_XX constants
    int32_t  set_dac(uint16_t dac); // not used actually in FPGA design
    uint16_t get_dac(void);         // not used actually in FPGA design

    int32_t  set_imem(uint8_t *myimem);
    uint32_t read_imem_file(FILE *f, uint8_t *prog_data, uint32_t max_length);

    int32_t  get_dpram(uint8_t *myram);
    int32_t  display_write(uint8_t reg0ram1, uint8_t wdata);
    int32_t  display_read(uint8_t reg0ram1);
    int32_t  display_on_off(uint8_t dis_on, uint8_t curs_on, uint8_t blink_on);
    int32_t  display_entry_set(uint8_t incr, uint8_t shift);
    int32_t  display_ret_home();
    int32_t  display_clear();
    int32_t  display_curs_dis_shift(uint8_t curs0_dis1, uint8_t left0_right1);
    int32_t  display_func_set(uint8_t byte1_nibble0, uint8_t L1_H2_lines, uint8_t L5x8_H5x11);
    int32_t  display_set_cg_addr(uint8_t addr);
    int32_t  display_set_dr_addr(uint8_t addr);
    int32_t  display_wr_data(uint8_t wdata);
    int32_t  display_rd_data();
    int32_t  display_rd_baddr();
    int32_t  display_wait_nbusy(uint16_t max_tries);


    int32_t  set_presamples(uint16_t pres);
    int32_t  set_trigger_tp(uint8_t adc_dis, uint8_t testpattern, uint8_t trigg_enable, uint16_t threshold);
    // timer will be modified in hardware!
    int32_t  get_timer(uint32_t *time_1Hz, uint32_t *time_10MHz);
    uint16_t get_presamples(void);
    uint16_t get_threshold(void);
    uint16_t get_ev_cnt(void);
    uint8_t  get_status(void);
    uint32_t get_svn_id(void);
    virtual uint32_t get_compile_id(void);
    uint32_t print_compile_id(FILE *f, uint32_t cmp_id);
    uint32_t print_compile_id(FILE *f);
    uint32_t print_svn_id(FILE *f, uint32_t svn_id);
    uint32_t print_svn_id(FILE *f);
    uint32_t print_program_id(FILE *f, uint32_t prg_id);
    uint32_t print_program_id(FILE *f);
    // use the last read status and extract some status bits
    uint8_t  last_eflash(void); // design loaded from external flash
    uint8_t  last_full(void);   // adc buffer is full
    uint8_t  last_empty(void);  // adc buffer is empty
    uint8_t  print_status(FILE *f);
    uint16_t get_event(uint16_t *adc); // get 4k samples
    uint16_t get_event(FILE *f);       // get 4k samples
    // functions which need EFB
    uint8_t  set_dac_i2c(uint16_t dac_dat);
    uint8_t  set_dacf_i2c(uint16_t dac_dat);   // write to flash of the DAC

    // for the control card only!
    uint32_t read_i2c_adc(uint8_t adc_ch);


    // EFB -> move to another class
    // public
    uint8_t  wb_reset();   // reset the WB bus access to flash Memory
    // write to the wishbone bus
    int32_t  write_wb(uint8_t wb_addr, uint8_t wb_data);
    // ... same, but more bytes
    int32_t  write_wb(uint8_t wb_addr, uint8_t nbytes, uint8_t *wb_data);
    // read from the wishbone bus
    uint8_t  read_wb(uint8_t wb_addr);
    // ... same, but more bytes
    uint8_t  read_wb(uint8_t wb_addr, uint8_t nbytes, uint8_t *rd_data);
    // I2C, may be add more functions?
    uint8_t  i2c_init(void);
    uint32_t read_i2c_2bytes(uint8_t i2c_slave);
    uint32_t wr1b_rd2b_i2c(uint8_t i2c_slave, uint8_t byte1);
    uint8_t  write_i2c_2bytes(uint8_t i2c_slave, uint8_t byte1, uint8_t byte2);
    uint8_t  write_i2c_3bytes(uint8_t i2c_slave, uint8_t byte1, uint8_t byte2, uint8_t byte3);

    // EFB functions to access the flash memory
    // read & check the device id
    uint32_t read_dev_id();
    // read the feature bits
    uint16_t read_feat_bits();
    // print the feature bits
    uint8_t  print_feat_bits(FILE *f, uint16_t fb);
    // read user code from SRAM (or from design)
    uint32_t read_user_code_sram();
    // read the user code from Flash Memory
    uint32_t read_user_code_flash();
    // program the user code (should be erased before)
    uint8_t  prog_user_code_flash(uint32_t ucode);
    // decode the device id as FPGA type
    uint8_t  print_dev_id(uint32_t dev_id, char *s);
    // returns the ufm pages (in bits 31..16) and config pages (in bits 15..0):
    uint32_t pages_in_dev(uint32_t dev_id);
    // same, but read the device id first from FPGA
    uint32_t pages_in_dev();
    // load the configuration from the internal flash, the FPGA design is for short time dead!
    uint8_t  wb_cfg_refresh();
    // read the configuration hex file, one byte per line
    uint32_t read_conf_file(FILE *f, uint8_t *prog_data, uint32_t max_length);
    // write the configuration hex file, one byte per line
    uint32_t write_conf_file(FILE *f, uint8_t *prog_data, uint32_t length);
    // read trace_id from the FPGA
    uint8_t  read_trace_id(uint8_t * trid);
    // read feature row from the FPGA
    uint8_t  read_feature_row(uint8_t *frow);
    // write internal flash, UFM or CFG and the user code
    uint8_t  wb_cfg_prog_flash(uint8_t ufm, uint32_t prog_size, uint8_t * byte_data, uint32_t ucode);
    // write internal flash, UFM or CFG without the user code
    uint8_t  wb_cfg_prog_flash(uint8_t ufm, uint32_t prog_size, uint8_t * byte_data);
    // read the internal flash, UFM or CFG
    uint8_t  wb_cfg_flash_rd(uint8_t ufm, uint32_t data_size, uint8_t *byte_data);
    // read page by page the internal flash, UFM or CFG - for debugging
    uint8_t  wb_cfg_flash_prd(uint8_t ufm, uint32_t data_size, uint8_t *byte_data);

    // EFB privat
    // check the device id
    uint8_t  check_dev_id(uint32_t dev_id, char *s);
    uint32_t wb_cnf_read(uint8_t cmd_code, uint32_t cmd_operand, uint8_t recv_length);
    uint8_t  wb_cnf_read(uint8_t cmd_code, uint32_t cmd_operand, uint32_t recv_length, uint8_t *recv_data);
    uint8_t  wb_cnf_write(uint8_t cmd_code, uint32_t cmd_operand, uint8_t send_length, uint32_t send_data);
    uint8_t  wb_cnf_write_arr(uint8_t cmd_code, uint32_t cmd_operand, uint32_t send_length, uint8_t *send_data);
    uint8_t  set_address_fm(uint8_t ufm, uint16_t addr);
    uint8_t  wb_cfg_reset_addr();
    uint8_t  wb_ufm_reset_addr();
    uint8_t  wb_cfg_enable();
    uint8_t  wb_cfg_disable();
    uint8_t  wb_cfg_bypass();
    uint8_t  wb_cfg_get_busy();
    uint8_t  wb_cfg_set_done();
    uint8_t  page_non_zero(uint8_t *page_data);
    uint32_t wb_cfg_get_status(uint8_t ret8bit_code, uint8_t show_bits);
    uint8_t  wb_cfg_erase(uint8_t erase_ufm, uint8_t erase_cfg,
                          uint8_t erase_fea, uint8_t erase_sram);
    uint8_t  wb_cfg_write_page(uint8_t ufm, uint8_t * byte_data);
    uint8_t  wb_wait_no_busy();

};

//const XO2DevInfo_t XO2DevList[LATTICE_XO2_NUM_DEVS] =
//{
//    // Name            Cfg pgs
//    //                  |     UFM pgs
//    //                  |       |   Cfg erase time
//    //                  |       |    |     UFM erase time
//    //                  |       |    |      |     Trefresh time
//    {"MachXO2-256",    575,     0,  700,    0,      1},
//    {"MachXO2-640",   1152,   191,  1100, 600,      1},
//    {"MachXO2-640U",  2175,   512,  1400, 700,      1},
//    {"MachXO2-1200",  2175,   512,  1400, 700,      1},
//    {"MachXO2-1200U", 3200,   639,  1900, 900,      2},
//    {"MachXO2-2000",  3200,   639,  1900, 900,      2},
//    {"MachXO2-2000U", 5760,   767,  3100, 1000,     3},
//    {"MachXO2-4000",  5760,   767,  3100, 1000,     3},
//    {"MachXO2-7000",  9216,  2046,  4800, 1600,     4}
//};

#endif /* ONE_IF033 */
