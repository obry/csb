// $Id: uart_test.cpp 68 2015-06-12 07:27:18Z angelov $:

#include <stdio.h>   /* Standard input/output definitions, for perror() */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions, for close() */
#include <fcntl.h>   /* File control definitions, for open() */
#include <termios.h> /* POSIX terminal control definitions */
#include <signal.h>  /* ANSI C signal handling */
#include <time.h>

#include <sys/time.h>
 //stty -F /dev/ttyUSB0 115200

#include "uart_crc.h"
#include "uart.h"
#include "one_if033.h"

#define DEVICE_MASTER "/dev/ttyUSB0"
#define DEF_BRATE 2000000
#define DEF_SLVID 7


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


uint32_t get_current_time()
{
    uint32_t prg_id;
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    prg_id = ( now->tm_sec << 24) | ( now->tm_min  << 18) | ( now->tm_hour  << 13) | (now->tm_mday  << 8) | ((now->tm_mon+1)     << 4) | ((now->tm_year+1900 - 2015) & 0xF);
    return prg_id;
}

#define debug_main 0


//int main(void)
int main(int argc, char** argv)
{
    uint8_t  cfg_data[MAX_CFG_DATA], adc_ch, imem_data[IMEM_SIZE*2];
    uart     *my_uart_pl;
    uart_crc *my_uart_crc;
    one_if033 *p_if033;
    uint16_t idx, slv_id, presamples, dac, testpatt, trigg_ena, threshold, i2c_not_init, adc_dis;
    uint32_t br, pings, nbytes, i;
    speed_t br_speed;
    FILE *f_out, *f_inp;
    char out_file[512];
    char inp_file[512];
    char dev_id[512];
    uint32_t ucode = 0;
    uint32_t ret;
    uint8_t my_ram[16], ch;

    strcpy(dev_id, DEVICE_MASTER); // default
    br = DEF_BRATE;
    slv_id = DEF_SLVID;

    i2c_not_init=0;


    if (argc < 2)
    {
        printf("program path: %s\n",argv[0]);
        print_usage();
        return(1);
    }
    else
    for (idx=1; idx<argc; idx++)
    {
        if (debug_main)
            printf("Current cmd line option is %s\n",argv[idx]);
        if (strcmp(argv[idx],"--dev") == 0 )
        {
            idx++;
            strcpy(dev_id, argv[idx]);
        }
        else
        if (strcmp(argv[idx],"--br") == 0 )
        {
            idx++;
            br = atoi(argv[idx]);
        }
        else
        if (strcmp(argv[idx],"--slv") == 0 )
        {
            idx++;
            slv_id = atoi(argv[idx]);
        }
    }
    br_speed = int2speed_t(br);
    my_uart_pl = new uart(dev_id, br_speed, 1);
    my_uart_crc = new uart_crc(my_uart_pl);
    p_if033 = new one_if033(0, my_uart_crc, slv_id);
    printf("UART %s open at speed %d\n", dev_id, br);

//    p_if033->print_program_id(stdout, get_current_time());

    for (idx=1; idx<argc; idx++)
    {
        if (debug_main)
            printf("Current cmd line option is %s\n",argv[idx]);
        if (strcmp(argv[idx],"--dev") == 0 )
        {
            idx++;
        }
        else
        if (strcmp(argv[idx],"--br") == 0 )
        {
            idx++;
        }
        else
        if (strcmp(argv[idx],"--slv") == 0 )
        {
            idx++;
        }
        else
        if (strcmp(argv[idx],"--pingc") == 0 )
        {
            idx++;
            pings = atoi(argv[idx]);
            printf("Sending %d times a counter\n",pings);
            for (i=0; i<pings; i++)
            {
                usleep(1000);
                my_uart_pl->WriteByte(i & 0xFF);
            }
        }
        else
        if (strcmp(argv[idx],"--pinga") == 0 )
        {
            idx++;
            pings = atoi(argv[idx]);
            printf("Sending %d times 0xAA 0x55\n",pings);
            for (i=0; i<pings; i++)
            {
                usleep(1000);
                my_uart_pl->WriteByte(0xAA);
                //usleep(1);
                my_uart_pl->WriteByte(0x55);
            }
        }
        else
        if (strcmp(argv[idx],"--pre") == 0 )
        {
            idx++;
            presamples = atoi(argv[idx]);
            printf("Set presamples to %d\n",presamples);
            p_if033->set_presamples(presamples);
        }
        else
        if (strcmp(argv[idx],"--dac") == 0 )
        {
            idx++;
            dac = atoi(argv[idx]);
            printf("Set dac to %d\n",dac);
            if (i2c_not_init)
            {
                p_if033->i2c_init();
                i2c_not_init=0;
            }
            ret=p_if033->set_dac_i2c(dac);
            printf("Return code 0x%02x\n",ret);
        }
        else
        if (strcmp(argv[idx],"--dacf") == 0 )
        {
            idx++;
            dac = atoi(argv[idx]);
            printf("Set the flash & dac register to %d\n",dac);
            if (i2c_not_init)
            {
                p_if033->i2c_init();
                i2c_not_init=0;
            }
            ret=p_if033->set_dacf_i2c(dac);
            printf("Return code 0x%02x\n",ret);
        }
        else
        if (strcmp(argv[idx],"--radc") == 0 )
        {
            idx++;
            adc_ch = atoi(argv[idx]);
            printf("Read the ADC channel %d\n",adc_ch);
            if (i2c_not_init)
            {
                p_if033->i2c_init();
                i2c_not_init=0;
            }
            ret=p_if033->read_i2c_adc(adc_ch);
            printf("Return code 0x%02x, adc read 0x%04x = %d = %0.3f V\n",ret >> 16, ret & 0xFFFF, ret & 0xFFF, 3.3*(ret & 0xFFF)/4096);
        }
        else
        if (strcmp(argv[idx],"--dist") == 0 )
        {
            p_if033->display_func_set(1, 1, 0);
            usleep(10000);
            p_if033->display_func_set(1, 1, 0);
            usleep(200);
            p_if033->display_func_set(1, 1, 0);
            if (p_if033->display_wait_nbusy(10) < 0) printf("Display timeout!\n");
            p_if033->display_on_off(0, 0, 0);
            if (p_if033->display_wait_nbusy(10) < 0) printf("Display timeout!\n");
            p_if033->display_clear();
            if (p_if033->display_wait_nbusy(10) < 0) printf("Display timeout!\n");
            p_if033->display_entry_set(1, 0);
            if (p_if033->display_wait_nbusy(10) < 0) printf("Display timeout!\n");
            p_if033->display_ret_home();
            if (p_if033->display_wait_nbusy(10) < 0) printf("Display timeout!\n");
            p_if033->display_on_off(1, 0, 0);
            if (p_if033->display_wait_nbusy(10) < 0) printf("Display timeout!\n");
            p_if033->display_set_dr_addr(0);
            if (p_if033->display_wait_nbusy(10) < 0) printf("Display timeout!\n");
            for (ch=0; ch<16; ch++)
            {
                if (ch==8)
                {
                    p_if033->display_set_dr_addr(0x40);
                    if (p_if033->display_wait_nbusy(10) < 0) printf("Display timeout!\n");
                }
                p_if033->display_wr_data(ch+0x30);
                if (p_if033->display_wait_nbusy(10) < 0) printf("Display timeout!\n");
            }
        }
        else
        if (strcmp(argv[idx],"--dacramp") == 0 )
        {
            printf("DAC ramp from 0 to 255\n");
            if (i2c_not_init)
            {
                p_if033->i2c_init();
                i2c_not_init=0;
            }
            for (dac=0; dac<4096; dac++)
            {
                p_if033->set_dac_i2c(dac);
            }
        }
        else
        if (strcmp(argv[idx],"--thresh") == 0 )
        {
            adc_dis  = atoi(argv[++idx]);
            testpatt = atoi(argv[++idx]);
            trigg_ena = atoi(argv[++idx]);
            threshold = atoi(argv[++idx]);
            printf("Set disable ADC to %d, test pattern to %d, trigger enable to %d and threshold to %d\n",adc_dis, testpatt, trigg_ena, threshold);
            p_if033->set_trigger_tp(adc_dis, testpatt, trigg_ena, threshold);
            p_if033->send_cmd(CMD_BUFF_CLR);
        }
        else
        if (strcmp(argv[idx],"--rdram") == 0 )
        {
            p_if033->get_dpram(my_ram);
        }
        else
        if (strcmp(argv[idx],"--trigg") == 0 )
        {
            p_if033->send_cmd(CMD_SOFT_TRIG);
        }
        else
        if (strcmp(argv[idx],"--reset") == 0 )
        {
            p_if033->send_cmd(CMD_SOFT_RST);
        }
        else
        if (strcmp(argv[idx],"--cpu_rst") == 0 )
        {
            p_if033->send_cmd(CMD_CPU_RST);
            p_if033->send_cmd(0);
        }
        else
        if (strcmp(argv[idx],"--bufclr") == 0 )
        {
            p_if033->send_cmd(CMD_BUFF_CLR);
        }
        else
        if (strcmp(argv[idx],"--timerclr") == 0 )
        {
            p_if033->send_cmd(CMD_TIMR_RST);
        }
        else
        if (strcmp(argv[idx],"-o") == 0 )
        {
            idx++;
            strcpy(out_file, argv[idx]);
            f_out = fopen(out_file,"w");
            p_if033->get_event(f_out);
        }
        else
        if (strcmp(argv[idx],"--status") == 0 )
        {
            p_if033->print_status(stdout);
        }
        else
        if (strcmp(argv[idx],"--ucode") == 0 )
        {
            idx++;
            if (strcmp(argv[idx],"timestamp") == 0 )
            {
                ucode = get_current_time();
            }
            // read the user code for subsequent CFG programming
            else
                read_hex_dec(argv[idx], &ucode);
        }
        else
        if (strcmp(argv[idx],"--rd_ufm") == 0 )
        {
            nbytes  = atoi(argv[++idx]);
            // auto length ?
            if (nbytes==0) nbytes = 16*(p_if033->pages_in_dev() >> 16);
            // open the file with the output data
            strcpy(out_file, argv[++idx]);
            f_out = fopen(out_file,"w");
            // read UFM
            p_if033->wb_cfg_flash_rd(1, nbytes, cfg_data);
            // store to a file
            for (i=0; i<nbytes; i++)
            {
                fprintf(f_out, "%02x",cfg_data[i]);
                if ((i & 0xF) == 0xF)
                    fprintf(f_out, "\n");
            }
            fclose(f_out);
        }
        else
        if (strcmp(argv[idx],"--rd_cfg") == 0 )
        {
            nbytes  = atoi(argv[++idx]);
            // auto length ?
            if (nbytes==0) nbytes = 16*(p_if033->pages_in_dev() & 0xFFFF)-32;
            // open the file with the output data
            strcpy(out_file, argv[++idx]);
            f_out = fopen(out_file,"w");
            // read CFG flash
            p_if033->wb_reset();
            p_if033->wb_cfg_flash_rd(0, nbytes, cfg_data);
            // store to a file
            p_if033->write_conf_file(f_out, cfg_data, nbytes);
            fclose(f_out);
        }
        else
        if (strcmp(argv[idx],"--er_ufm") == 0 )
        {
            p_if033->wb_reset();
            p_if033->wb_cfg_prog_flash(1, 0, NULL);
        }
        else
        if (strcmp(argv[idx],"--er_cfg") == 0 )
        {
            p_if033->wb_reset();
            p_if033->wb_cfg_prog_flash(0, 0, NULL);
        }
        else
        if (strcmp(argv[idx],"--wr_ufm") == 0 )
        {
            nbytes  = atoi(argv[++idx]);
            // auto length ?
            if (nbytes==0) nbytes = 16*(p_if033->pages_in_dev() >> 16);
            // open the file with the output data, later input
            strcpy(out_file, argv[++idx]);
            f_out = fopen(out_file,"w");
            // generate pseudorandom data
            cfg_data[0] = 0xEF;
            for (i=1; i<nbytes; i++)
            {
                cfg_data[i] = gen_psrg(cfg_data[i-1]);
            }
            // write to a file and eventually clear some pages
            for (i=0; i<nbytes; i++)
            {
                // some pages with 00000
//                if ( (i > 63) && (i < (nbytes-128) ) ) cfg_data[i] = 0;
                fprintf(f_out, "%02x",cfg_data[i]);
                if ((i & 0xF) == 0xF)
                    fprintf(f_out, "\n");
            }
            fclose(f_out);
            // program the UFM
            p_if033->wb_reset();
            p_if033->wb_cfg_prog_flash(1, nbytes, cfg_data);
        }
        else
        if (strcmp(argv[idx],"--wr_cfg") == 0 )
        {
            nbytes  = atoi(argv[++idx]);
            // auto length ?
            if (nbytes==0) nbytes = 16*(p_if033->pages_in_dev() & 0xFFFF)-32;
            // clear the array for config data
            for (i=0; i<nbytes; i++) cfg_data[i] = 0;
            // open the file with the config for reading
            strcpy(inp_file, argv[++idx]);
            f_inp = fopen(inp_file,"r");
            // read the config
            p_if033->read_conf_file(f_inp, cfg_data, nbytes);
            fclose(f_inp);
            // write the config to the internal flash
            p_if033->wb_reset();
            p_if033->wb_cfg_prog_flash(0, nbytes, cfg_data, ucode);
        }
        else
        if (strcmp(argv[idx],"--wr_imem") == 0 )
        {
            p_if033->send_cmd(CMD_CPU_RST);
            // open the file with the config for reading
            strcpy(inp_file, argv[++idx]);
            f_inp = fopen(inp_file,"r");
            // read the config
            p_if033->read_imem_file(f_inp, imem_data, IMEM_SIZE);
            fclose(f_inp);
            // write to the IMEM
            p_if033->set_imem(imem_data);
            // CPU reset
            p_if033->send_cmd(0);
        }
        else
        {
            printf("Unknown command line option %s\n",argv[idx]);
            print_usage();
            return(1);
        }
    }

    printf("Bytes sent/received %d / %d\n", my_uart_pl->bytes_sent, my_uart_pl->bytes_received);
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
