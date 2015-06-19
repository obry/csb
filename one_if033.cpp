#include "iostream"
#include "ctime"
#include "uart_crc.h"
#include "one_if033.h"

one_if033::one_if033(uint8_t new_debug, uart_crc *p_uart_crc, uint8_t new_slave_addr)
{
    m_uart_crc = p_uart_crc;
    debug = new_debug;
    slave_addr = new_slave_addr;
    last_status = 0xFF; // unknown
}

one_if033::~one_if033()
{
    // do nothing
}



// specific to IF033 - one channel
int32_t one_if033::send_cmd(uint8_t cmd)
{
    if (debug)
    {
        printf("Sending to COMMAND reg 0x%02x\n",cmd);
    }
    return m_uart_crc->WriteByte(slave_addr, ADDR_COMMAND | AUTOINC_ADDR, cmd, false);
}

int32_t one_if033::set_dac(uint16_t dac) // not used actually in FPGA design
{
    if (debug)
        printf("Sending to DAC reg 0x%04x\n",dac);
    return m_uart_crc->WriteWord(slave_addr, ADDR_DAC | AUTOINC_ADDR, dac, false);
}

int32_t one_if033::set_presamples(uint16_t pres)
{
    if (debug)
        printf("Sending to PRESAMPLES reg 0x%04x\n",pres);
    return m_uart_crc->WriteWord(slave_addr, ADDR_PRESAMPLES | AUTOINC_ADDR, pres, false);
}
//                                disable adc    | generate testpattern | enable trigger   | set threshold
int32_t one_if033::set_trigger_tp(uint8_t adc_dis, uint8_t testpattern, uint8_t trigg_enable, uint16_t threshold)
{
    uint16_t wrk;
    threshold &= 0x0FFF;
    wrk = adc_dis & 1;
    wrk <<= 1;
    wrk |= testpattern & 1;
    wrk <<= 1;
    wrk |= trigg_enable & 1;
    wrk <<= 12;
    wrk |= threshold & 0x0FFF;
 //   if (debug)
        printf("Sending to THRESHOLD reg 0x%04x\n",wrk);
    return m_uart_crc->WriteWord(slave_addr, ADDR_THRESHOLD | AUTOINC_ADDR, wrk, false);
}

int32_t one_if033::get_timer(uint32_t *time_1Hz, uint32_t *time_10MHz)
{
    uint32_t tm_1Hz;
    uint32_t tm_10MHz;
    uint8_t my_bytes[8];
    int32_t ret;

    ret=m_uart_crc->RecvBurst(slave_addr, 5, ADDR_TIMER_10MHZ | AUTOINC_ADDR, my_bytes, false);

    tm_1Hz = my_bytes[4];
    tm_1Hz <<= 8;
    tm_1Hz |= my_bytes[3];
    tm_10MHz = my_bytes[2];
    tm_10MHz <<= 8;
    tm_10MHz |= my_bytes[1];
    tm_10MHz <<= 8;
    tm_10MHz |= my_bytes[0];
    *time_1Hz = tm_1Hz;
    *time_10MHz = tm_10MHz;
    return ret;
}

int32_t one_if033::get_dpram(uint8_t *myram)
{
    int32_t ret, i;
    ret=m_uart_crc->RecvBurst(slave_addr, 16, ADDR_DPRAM | AUTOINC_ADDR, myram, false);
    for (i=0; i<16; i++)
        printf(" 0x%02x",myram[i])  ;
    printf("\n");
    return ret;
}

int32_t one_if033::set_imem(uint8_t *myimem)
{
    int32_t ret, i;
    ret=m_uart_crc->SendBurst(slave_addr, IMEM_SIZE*2, ADDR_IMEM | AUTOINC_ADDR, myimem, false);
    return ret;
}

uint16_t one_if033::get_presamples(void)
{
    return m_uart_crc->ReadWord(slave_addr, ADDR_PRESAMPLES | AUTOINC_ADDR, false);
}

uint16_t one_if033::get_threshold(void)
{
    return m_uart_crc->ReadWord(slave_addr, ADDR_THRESHOLD | AUTOINC_ADDR, false);
}

uint16_t one_if033::get_dac(void) // not used actually in FPGA design
{
    return m_uart_crc->ReadWord(slave_addr, ADDR_DAC | AUTOINC_ADDR, false);
}

uint16_t one_if033::get_ev_cnt(void)
{
    return m_uart_crc->ReadWord(slave_addr, ADDR_EV_CNT | AUTOINC_ADDR, false);
}

uint8_t one_if033::get_status(void)
{
    last_status = m_uart_crc->ReadByte(slave_addr, ADDR_STATUS | AUTOINC_ADDR, false);
    return last_status;
}

uint8_t  one_if033::last_eflash(void)
{
    if (last_status & STATUS_EFLASH) return 1;
    else return 0;
}

uint8_t  one_if033::last_full(void)
{
    if (last_status & STATUS_FULL) return 1;
    else return 0;
}

uint8_t  one_if033::last_empty(void)
{
    if (last_status & STATUS_EMPTY) return 1;
    else return 0;
}

uint32_t one_if033::get_svn_id(void)
{
    return m_uart_crc->Read3Bytes(slave_addr, ADDR_SVN | AUTOINC_ADDR, false);
}

uint32_t one_if033::get_compile_id(void)
{
    return m_uart_crc->Read3Bytes(slave_addr, ADDR_COMPILE | AUTOINC_ADDR, false);
}

uint32_t one_if033::print_compile_id(FILE *f, uint32_t cmp_id)
{
   fprintf(f, "# Compiled on %4d-%02d-%02d at %02d:%02d\n", (cmp_id       & 0x0F) + 2015,
                                                            (cmp_id >> 4) & 0x0F,
                                                            (cmp_id >> 8) & 0x1F,
                                                            (cmp_id >>13) & 0x1F,
                                                            (cmp_id >>18) & 0x3F);
    return cmp_id;
}

uint32_t one_if033::print_program_id(FILE *f, uint32_t prg_id)
{
    fprintf(f, "# Programmed on %4d-%02d-%02d at %02d:%02d:%02d\n", (prg_id       & 0x0F) + 2015,
                                                             (prg_id >> 4) & 0x0F,
                                                             (prg_id >> 8) & 0x1F,
                                                             (prg_id >>13) & 0x1F,
                                                             (prg_id >>18) & 0x3F,
                                                             (prg_id >>24) & 0x3F);
    return prg_id;
}

uint32_t one_if033::print_program_id(FILE *f)
{
    return print_program_id(f, read_user_code_flash());
}

uint32_t one_if033::print_compile_id(FILE *f)
{
    return print_compile_id(f, get_compile_id());
}

uint32_t one_if033::print_svn_id(FILE *f, uint32_t svn_id)
{
    fprintf(f, "# SVN revision %d from %4d-%02d-%02d\n",     (svn_id >>13) & 0x7FF,
                                                             (svn_id       & 0x0F) + 2015,
                                                             (svn_id >> 4) & 0x0F,
                                                             (svn_id >> 8) & 0x1F);
    return svn_id;
}

uint32_t one_if033::print_svn_id(FILE *f)
{
    return print_svn_id(f, get_svn_id());
}



uint8_t one_if033::print_status(FILE *f)
{
    uint8_t trid[8], frow[8], i;
    uint16_t dac, threshold, presamples, ev_cnt;
    uint32_t tm_10MHz, tm_1Hz, fpga_dev_id, user_code;
    char dev_name[128];
    presamples = get_presamples();
    dac = get_dac();
    threshold = get_threshold();
    ev_cnt = get_ev_cnt();
    get_timer(&tm_1Hz, &tm_10MHz);
    get_status();
    fpga_dev_id = read_dev_id();
    user_code = read_user_code_sram();
    print_dev_id(fpga_dev_id, dev_name);
    read_trace_id(trid);
    read_feature_row(frow);
    fprintf(f, "# FPGA device ID is 0x%08x, ",fpga_dev_id);
    fprintf(f, "FPGA is %s\n", dev_name);
    fprintf(f, "# USERCODE in design is 0x%08x\n",user_code);
    fprintf(f, "# USERCODE in flash  is 0x%08x\n",read_user_code_flash());
    print_program_id(f);
    fprintf(f, "# TRACE_ID is:");
    for (i=0; i<8; i++) fprintf(f, " %02x",trid[i]);
    fprintf(f, "\n");

    fprintf(f, "# FEATURE_ROW is:");
    for (i=0; i<8; i++) fprintf(f, " %02x",frow[i]);
    fprintf(f, "\n");

    print_feat_bits(f, read_feat_bits());

    if (last_eflash())
        fprintf(f, "# Design load from external flash!!!\n");
    else
        fprintf(f, "# Design load from internal flash\n");
    print_svn_id(f);
    print_compile_id(f);
    fprintf(f, "# DAC is 0x%04x\n",dac);
    fprintf(f, "# PRESAMPLES is 0x%04x\n",presamples);
    fprintf(f, "# THRESHOLD is 0x%03x\n",threshold & 0xFFF);
    fprintf(f, "# TESTPATTERN is %d, TRIGGER_ENABLE is %d\n",(threshold >> 13) & 1, (threshold >> 12) & 1);
    fprintf(f, "# FULL is %d, EMPTY is %d\n",last_full(), last_empty());
    fprintf(f, "# Last event # %d, timer %d [s] and %d [100ns]\n", ev_cnt, tm_1Hz, tm_10MHz);
    return last_status;
}

uint16_t one_if033::get_event(uint16_t *adc) // get 4k samples
{
    uint8_t buff[3*ADC_SAMPLES/2+10];
    uint16_t sampl, temp, buf_idx;

    // read 3x 2048 bytes without antoincrement
    m_uart_crc->RecvBurst(slave_addr, 2048, ADDR_EV_BUFFER,  buff,                false);
    m_uart_crc->RecvBurst(slave_addr, 2048, ADDR_EV_BUFFER, &buff[ADC_SAMPLES/2], false);
    m_uart_crc->RecvBurst(slave_addr, 2048, ADDR_EV_BUFFER, &buff[ADC_SAMPLES  ], false);

    sampl = 0;
    buf_idx = 0;
    // convert to 12-bit ADC values
    do
    {
        // sample 0
        adc[sampl] = buff[buf_idx++];
        temp = buff[buf_idx++];
        adc[sampl++] |= (temp & 0xF) << 8;
        // sample 1
        adc[sampl] = temp >> 4;
        temp = buff[buf_idx++];
        adc[sampl++] |= temp << 4;
    } while ((sampl < ADC_SAMPLES) || (buf_idx < 3*2048) );
    return 0;
}

uint16_t one_if033::get_event(FILE *f) // get 4k samples
{
    uint16_t adc[ADC_SAMPLES], i;
    print_status(f);
    get_event(adc);
    for (i=0; i<ADC_SAMPLES; i++)
        fprintf(f, "%4d %4d\n",i,adc[i]);
    fclose(f);
    return 0;
}


// EFB

int32_t  one_if033::write_wb(uint8_t wb_addr, uint8_t wb_data)
{
    if (debug)
        printf("writing to addr 0x%02x data 0x%02x\n", wb_addr, wb_data);
    return m_uart_crc->WriteByte(slave_addr, wb_addr, wb_data, false);
}

int32_t  one_if033::write_wb(uint8_t wb_addr, uint8_t nbytes, uint8_t *wb_data)
{
    if (debug)
        printf("writing a burst of %d bytes to addr 0x%02x\n", nbytes, wb_addr);
    return m_uart_crc->SendBurst(slave_addr, nbytes, wb_addr & NO_AUTOINC_ADDR, wb_data, false);
}

uint8_t  one_if033::read_wb(uint8_t wb_addr)
{
    return m_uart_crc->ReadByte(slave_addr, wb_addr, false);
}

uint8_t  one_if033::read_wb(uint8_t wb_addr, uint8_t nbytes, uint8_t *rd_data)
{
    return m_uart_crc->RecvBurst(slave_addr, nbytes, wb_addr & NO_AUTOINC_ADDR, rd_data, false);
}

uint8_t one_if033::i2c_init(void)
{
    uint8_t rd0, rd1, rd2, rd3;
    write_wb(I2C2_CR, 0x80);
 //   if (debug)
    {
        rd0 = read_wb(I2C2_BR0);
        rd1 = read_wb(I2C2_BR1);
        rd2 = read_wb(I2C2_SR);
        rd3 = read_wb(I2C2_CR);
        printf("BR0 is 0x%02x, BR1 is 0x%02x, status is 0x%02x, control is 0x%02x\n",
                rd0, rd1, rd2, rd3);
    }
    //sleep(1);
    return 0;
}

uint32_t one_if033::read_i2c_2bytes(uint8_t i2c_slave)
{
    uint32_t res;
    uint8_t my_bytes[2];
    write_wb(I2C2_TXDR, i2c_slave | 1);
    my_bytes[0] = 0x90; // STA+WR
    my_bytes[1] = 0x20; // RD
    write_wb(I2C2_CMDR, 2, my_bytes);
//    write_wb(I2C2_CMDR, 0x90); // STA+WR
//    write_wb(I2C2_CMDR, 0x20); // RD
    res = read_wb(I2C2_RXDR);  // upper 8 bits
    my_bytes[0] = 0x20; // RD
    my_bytes[1] = 0x68; // RD+NACK+STOP
    write_wb(I2C2_CMDR, 2, my_bytes);
//    write_wb(I2C2_CMDR, 0x20); // RD
//    write_wb(I2C2_CMDR, 0x68); // RD+NACK+STOP
    res <<= 8;
    res |= read_wb(I2C2_RXDR); // lower 8 bits
    res |= read_wb(I2C2_SR) << 16; // status
    return res;
}

uint32_t one_if033::wr1b_rd2b_i2c(uint8_t i2c_slave, uint8_t byte1)
{
    uint32_t res;
    uint8_t my_bytes[2];

    write_wb(I2C2_TXDR, i2c_slave & 0xFE);
    write_wb(I2C2_CMDR, 0x90); // STA+WR

    my_bytes[0] = byte1;
    my_bytes[1] = i2c_slave | 1;
    write_wb(I2C2_TXDR, 2, my_bytes);
//    write_wb(I2C2_TXDR, byte1 );
//    write_wb(I2C2_TXDR, i2c_slave | 1);

    my_bytes[0] = 0x90; // STA+WR
    my_bytes[1] = 0x20; // RD
    write_wb(I2C2_CMDR, 2, my_bytes);
//  write_wb(I2C2_CMDR, 0x90); // STA+WR
//  write_wb(I2C2_CMDR, 0x20); // RD
    res = read_wb(I2C2_RXDR);  // upper 8 bits
    res <<= 8;
    write_wb(I2C2_CMDR, 0x68); // RD+NACK+STOP
    res |= read_wb(I2C2_RXDR); // lower 8 bits
    res |= read_wb(I2C2_SR) << 16; // status
    return res;
}

uint8_t one_if033::write_i2c_2bytes(uint8_t i2c_slave, uint8_t byte1, uint8_t byte2)
{
    uint8_t my_bytes[2];

    write_wb(I2C2_TXDR, i2c_slave & 0xFE);
    write_wb(I2C2_CMDR, 0x90); // STA+WR
    write_wb(I2C2_TXDR, byte1 );
    write_wb(I2C2_CMDR, 0x10); // WR
    write_wb(I2C2_TXDR, byte2);

    my_bytes[0] = 0x10; // WR
    my_bytes[1] = 0x40; // STOP
    write_wb(I2C2_CMDR, 2, my_bytes);

//  write_wb(I2C2_CMDR, 0x10); // WR
//  write_wb(I2C2_CMDR, 0x40); // STOP
    return read_wb(I2C2_SR);
}

uint32_t one_if033::read_i2c_adc(uint8_t adc_ch)
{
    adc_ch &= 0xF;

//    return wr1b_rd2b_i2c(ADC_SLV_ADDR, 0x61 | (adc_ch << 1));


    //                          Setup: ext clock, no reset  Config: no scan, se
    write_i2c_2bytes(ADC_SLV_ADDR, 0x80 | 0x08 | 0x02,              0x61 | (adc_ch << 1));
//    return 0;
//    sleep(2);
    return read_i2c_2bytes(ADC_SLV_ADDR);
}

uint8_t one_if033::write_i2c_3bytes(uint8_t i2c_slave, uint8_t byte1, uint8_t byte2, uint8_t byte3)
{
    uint8_t my_bytes[2];

    write_wb(I2C2_TXDR, i2c_slave & 0xFE);
    write_wb(I2C2_CMDR, 0x90); // STA+WR
    write_wb(I2C2_TXDR, byte1 );
    write_wb(I2C2_CMDR, 0x10); // WR
    write_wb(I2C2_TXDR, byte2);
    write_wb(I2C2_CMDR, 0x10); // WR
    write_wb(I2C2_TXDR, byte3);

    my_bytes[0] = 0x10; // WR
    my_bytes[1] = 0x40; // STOP
    write_wb(I2C2_CMDR, 2, my_bytes);

//  write_wb(I2C2_CMDR, 0x10); // WR
//  write_wb(I2C2_CMDR, 0x40); // STOP
    return read_wb(I2C2_SR);
}

uint8_t one_if033::set_dac_i2c(uint16_t dac_dat)
{
    uint8_t byte1, byte2;
    byte1 = 0; // power down
    dac_dat &= 0xFFF;
    // for 10-bit DAC: dac_dat <<= 2;
    byte1 = (byte1 << 4) | (dac_dat >> 8);
    byte2 =  dac_dat & 0xFF;
    return write_i2c_2bytes(DAC_SLV_ADDR, byte1, byte2);
}

uint8_t one_if033::set_dacf_i2c(uint16_t dac_dat)
{
    uint8_t gain;
    uint8_t pdown;
    uint8_t vref;
    uint8_t cmnd;
    uint8_t byte1, byte2, byte3;

    gain  = 0; // 1 bit, 0 for gain 1, and 1 for gain 2
    pdown = 0; // 2 bits, 0 for normal
    vref  = 3; // 2 bits, "11" for external buffered VREF
    cmnd  = 3; // 3 bits, "011" for write to all
//    dac_dat <<= 8; // for 8-bit chip
    dac_dat <<= 4; // for 12-bit chip

    // build the command byte
    byte1 = cmnd;
    byte1 <<= 2;
    byte1 |= vref & 0x03;
    byte1 <<= 2;
    byte1 |= pdown & 0x03;
    byte1 <<= 1;
    byte1 |= gain & 0x01;

    byte2 = (dac_dat >> 8) & 0xFF;
    byte3 =  dac_dat       & 0xFF;

    return write_i2c_3bytes(DAC_SLV_ADDR, byte1, byte2, byte3);
}

//                                 HE/ZE                   HC
// MachXO2-256                     0x01 2B 00 43       0x01 2B 80 43
// MachXO2-640                     0x01 2B 10 43       0x01 2B 90 43
// MachXO2-1200/MachXO2-640U       0x01 2B 20 43       0x01 2B A0 43
// MachXO2-2000/MachXO2-1200U      0x01 2B 30 43       0x01 2B B0 43
// MachXO2-4000/MachXO2-2000U      0x01 2B 40 43       0x01 2B C0 43
// MachXO2-7000                    0x01 2B 50 43       0x01 2B D0 43
uint8_t one_if033::check_dev_id(uint32_t dev_id, char *s)
{
    uint8_t const_bytes_ok = 1;
    char message[128];

    strcpy(message, "");


    if ( (dev_id >> 24)          != 0x01)
    {
        strcat(message, "MSByte expected 0x01!\n");
        const_bytes_ok = 0;
    }

    if ( ((dev_id >> 16) & 0xFF) != 0x2B)
    {
        strcat(message, "Bits23..16 expected 0x2B!\n");
        const_bytes_ok = 0;
    }

    if ( ((dev_id >> 8) & 0xF) != 0x0)
    {
        strcat(message, "Bits11..8 expected 0000!\n");
        const_bytes_ok = 0;
    }

    if ( ((dev_id >> 12) & 0x7) > 5)
    {
        strcat(message, "Bits14..12 > 5!\n");
        const_bytes_ok = 0;
    }

    if ( ( dev_id        & 0xFF) != 0x43)
    {
        strcat(message, "LSByte expected 0x43!\n");
        const_bytes_ok = 0;
    }

    strcpy(s, message);

    if (const_bytes_ok == 0)
        return 1;
    else
        return 0;
}

uint8_t one_if033::print_dev_id(uint32_t dev_id, char *s)
{
    char dev_name[128];

    if (check_dev_id(dev_id, dev_name))
    {
        printf("%s\n",dev_name);
        strcpy(s, dev_name);
        return 0;
    }

    switch ((dev_id >> 12) & 0x7)
    {
    case 0x0 : { strcpy(dev_name, "MachXO2-256");  break; }
    case 0x1 : { strcpy(dev_name, "MachXO2-640"); break; }
    case 0x2 : { strcpy(dev_name, "MachXO2-1200/640U"); break; }
    case 0x3 : { strcpy(dev_name, "MachXO2-2000/1200U"); break; }
    case 0x4 : { strcpy(dev_name, "MachXO2-4000/2000U"); break; }
    case 0x5 : { strcpy(dev_name, "MachXO2-7000"); break; }
    }
    if ((dev_id >> 15) & 1) strcat(dev_name, "-HC");
    else                    strcat(dev_name, "-HE/ZE");

    strcpy(s, dev_name);

    return (dev_id >> 12) & 0xF;

}

uint32_t one_if033::pages_in_dev()
{
    return pages_in_dev(read_dev_id());
}

uint32_t one_if033::pages_in_dev(uint32_t dev_id)
{
    char s[128];

    if (check_dev_id(dev_id, s))
    {
        printf("%s\n",s);
        return 0;
    }

    switch ((dev_id >> 12) & 0x7)
    {
    case 0x0 : return                 575; // MachXO2-256
    case 0x1 : return ( 191 << 16) | 1152; // MachXO2-640
    case 0x2 : return ( 512 << 16) | 2175; // MachXO2-1200/640U
    case 0x3 : return ( 639 << 16) | 3200; // MachXO2-2000/1200U
    case 0x4 : return ( 767 << 16) | 5760; // MachXO2-4000/2000U
    case 0x5 : return (2046 << 16) | 9216; // MachXO2-7000
    }
    return 0;
}

uint8_t one_if033::wb_reset()
{
    uint8_t my_bytes[2];

    my_bytes[0] = 0x40; // set the reset bit 6
    my_bytes[1] = 0x00; // clear the reset bit
    write_wb(WB_CFGCR, 2, my_bytes);

//  write_wb(WB_CFGCR,   0x40);  // set the reset bit 6
//  write_wb(WB_CFGCR,   0x00);  // clear the reset bit
    return 0;
}

uint32_t one_if033::wb_cnf_read(uint8_t cmd_code, uint32_t cmd_operand, uint8_t recv_length)
{
    uint32_t longw;
    uint8_t my_bytes[4], j;

    longw = 0;

    write_wb(WB_CFGCR,   0x80);  // start frame

    my_bytes[0] = cmd_code;
    my_bytes[1] = (cmd_operand >> 16) & 0xFF;
    my_bytes[2] = (cmd_operand >>  8) & 0xFF;
    my_bytes[3] =  cmd_operand        & 0xFF;

    write_wb(WB_CFGTXDR, 4, my_bytes);

    // read 4 bytes
    if (recv_length > 4) recv_length = 4;
    if (recv_length < 1) recv_length = 1;

    read_wb(WB_CFGRXDR, recv_length, my_bytes);

    j = 0;
    if (recv_length > 3)
        longw |= my_bytes[j++] << 24;
    if (recv_length > 2)
        longw |= my_bytes[j++] << 16;
    if (recv_length > 1)
        longw |= my_bytes[j++] <<  8;

    longw |= my_bytes[j++] <<  0;

    write_wb(WB_CFGCR,   0x00);  // stop frame

    return longw;
}

uint8_t one_if033::wb_cnf_read(uint8_t cmd_code, uint32_t cmd_operand, uint32_t recv_length, uint8_t *recv_data)
{
    uint32_t i;
    uint8_t send_data[8];

    write_wb(WB_CFGCR,   0x80);  // start frame

    i = 0;
    send_data[i++] = cmd_code;
    send_data[i++] = (cmd_operand >> 16) & 0xFF;
    send_data[i++] = (cmd_operand >>  8) & 0xFF;
    send_data[i++] =  cmd_operand        & 0xFF;
    write_wb(WB_CFGTXDR, 4, send_data);

    if ( ( (cmd_code == 0x73) || (cmd_code == 0xCA) ) && ( (cmd_operand & 0x3FFF) > 1) ) // read flash and more than one page
    {
        //printf("Skip one page\n");
        read_wb(WB_CFGRXDR, 16, recv_data);
    }
    if ( (cmd_code == 0x73) || (cmd_code == 0xCA) ) // read flash
    {
        recv_length >>= 4; // convert to number of pages, 1 page = 16 bytes
        // read the bytes
        for (i = 0; i < recv_length; i++)
        {
            read_wb(WB_CFGRXDR, 16, recv_data);
            recv_data += 16;
        }
    }
    else
//    if (recv_length == 8)
        read_wb(WB_CFGRXDR, recv_length, recv_data);

    write_wb(WB_CFGCR,   0x00);  // stop frame

    return 0;
}

uint32_t one_if033::read_dev_id()
{
    return wb_cnf_read(0xE0, 0x000000, 4);
}

uint32_t one_if033::read_user_code_sram()
{
    return wb_cnf_read(0xC0, 0x000000, 4);
}

uint8_t one_if033::read_trace_id(uint8_t *trid)
{
    return wb_cnf_read(0x19, 0x000000, 8, trid);
}

uint8_t one_if033::read_feature_row(uint8_t *frow)
{
    wb_cfg_enable();
    wb_cnf_read(0xE7, 0x000000, 8, frow);
    wb_cfg_disable();
    return 0;
}

uint8_t one_if033::wb_cnf_write(uint8_t cmd_code, uint32_t cmd_operand, uint8_t send_length, uint32_t send_data)
{
    uint8_t i, j, operand_length = 3;
    uint8_t my_bytes[8];
    write_wb(WB_CFGCR,   0x80); // start frame

    j = 0;
    my_bytes[j++] = cmd_code;

    if ( (cmd_code == 0x26) || (cmd_code == 0x79) ) operand_length = 2;

    if (operand_length == 3)
        my_bytes[j++] = (cmd_operand >> 16) & 0xFF;
    my_bytes[j++] = (cmd_operand >> 8) & 0xFF;
    my_bytes[j++] =  cmd_operand       & 0xFF;

    if (send_length > 4) send_length = 4;

    for (i=send_length; i>0; i--)
        my_bytes[j++] = (send_data >> (8*(i-1) ) ) & 0xFF;

    write_wb(WB_CFGTXDR, j, my_bytes);

    write_wb(WB_CFGCR,   0x00); // stop frame

    return 0;
}


uint8_t  one_if033::prog_user_code_flash(uint32_t ucode)
{
    wb_cnf_write(0xC2, 0x000000, 4, ucode);
    return wb_wait_no_busy();
}

uint8_t one_if033::wb_cnf_write_arr(uint8_t cmd_code, uint32_t cmd_operand, uint32_t send_length, uint8_t *send_data)
{
    uint8_t i;
    uint8_t send_cmd[8];

    write_wb(WB_CFGCR,   0x80); // start frame

    i = 0;
    send_cmd[i++] = cmd_code;
    send_cmd[i++] = (cmd_operand >> 16) & 0xFF;
    send_cmd[i++] = (cmd_operand >>  8) & 0xFF;
    send_cmd[i++] =  cmd_operand        & 0xFF;
    write_wb(WB_CFGTXDR, 4, send_cmd);

    write_wb(WB_CFGTXDR, send_length, send_data);

    return 0;
}


// ufm=1 UFM, ufm=0 CFG
uint8_t one_if033::set_address_fm(uint8_t ufm, uint16_t addr)
{
    uint32_t lword;
    lword = (ufm & 1) << 6; // 0 or 1 at bit 6
    lword <<= 24;
    lword |= addr & 0x3FFF;  // 14-bit address

    return wb_cnf_write(0xB4, 0x000000, 4, lword);
}


uint8_t one_if033::wb_cfg_reset_addr()
{
    return wb_cnf_write(0x46, 0x000000, 0, 0);
}

uint8_t one_if033::wb_ufm_reset_addr()
{
    return wb_cnf_write(0x47, 0x000000, 0, 0);
}

uint8_t one_if033::wb_cfg_enable()
{
    return wb_cnf_write(0x74, 0x080000, 0, 0);
}

uint8_t one_if033::wb_cfg_disable()
{
    if ( (wb_cfg_get_status(0, 0) >> 12) & 1 )
        printf("Trying to disable the cfg interface while busy is 1!!!\n");
    wb_cnf_write(0x26, 0x0000, 0, 0);
    wb_cfg_bypass();

    return 0;
}

uint8_t one_if033::wb_cfg_bypass()
{
    return wb_cnf_write(0xFF, 0xFFFFFF, 0, 0);
}

uint32_t one_if033::wb_cfg_get_status(uint8_t ret8bit_code, uint8_t show_bits)
// bit  8 -> bit 0 Done
// bit  9 -> bit 1 Config enable
// bit 12 -> bit 2 Busy flag
// bit 13 -> bit 3 Fail flag
// bit 25..23 -> bit 6..4 ERR code
// bit 27 -> bit 7 I - device verified correct (0) or failed (1), see the ERR code
{
    uint32_t status;
    uint8_t stat8;
    status = wb_cnf_read(0x3C, 0x000000, 4);
    write_wb(WB_CFGCR,   0x00);  // stop frame

    if (show_bits)
    printf("Status read, Done=%d, ConfEna=%d, Busy=%d, Fail=%d, I=%d, EEE=%d\n",
        (status >>  8) & 1,
        (status >>  9) & 1,
        (status >> 12) & 1,
        (status >> 13) & 1,
        (status >> 27) & 1,
        (status >> 23) & 7);

    if (ret8bit_code == 0) return status;

    stat8  = (status >>  8) & 0x03;
    stat8 |= (status >> 10) & 0x0C;
    stat8 |= (status >> 19) & 0x70;
    stat8 |= (status >> 20) & 0x80;

    return stat8;
}

uint8_t one_if033::wb_cfg_get_busy()
{
    return wb_cnf_read(0xF0, 0x000000, 1) & 0xFF;
}

uint8_t one_if033::wb_cfg_erase(uint8_t erase_ufm, uint8_t erase_cfg,
                                uint8_t erase_fea, uint8_t erase_sram)
{
    uint32_t lword;

    erase_ufm &= 1;
    erase_cfg &= 1;
    erase_fea &= 1;
    erase_sram &= 1;
    lword = (erase_ufm << 3) | (erase_cfg << 2) | (erase_fea << 1) | erase_sram;
    lword <<= 16;
    return wb_cnf_write(0x0E, lword, 0, 0);
}

uint8_t one_if033::wb_cfg_write_page(uint8_t ufm, uint8_t *bit_data)
{
    uint8_t cmd_code;

    ufm &= 1;
    if (ufm) cmd_code = 0xC9;
    else     cmd_code = 0x70;
    wb_cnf_write_arr(cmd_code, 0x000001, 16, bit_data);
    usleep(200);
    return wb_wait_no_busy();
}

uint8_t one_if033::wb_cfg_set_done()
{
    wb_cnf_write(0x5E, 0x000000, 0, 0);
    return wb_wait_no_busy();
}

uint8_t one_if033::wb_cfg_refresh()
{
    return wb_cnf_write(0x79, 0x000000, 0, 0);
}

uint8_t one_if033::wb_wait_no_busy()
{
    uint16_t cnt = 0;
    uint8_t busy;

    do
    {
        busy = wb_cfg_get_busy();
//        printf("Waiting for busy=0, last read 0x%02x\n", busy);
        cnt++;
    } while ( (busy & 0x80) && (cnt < WB_MAX_BUSY) );
    return (busy >> 7) & 1;
}

uint8_t one_if033::page_non_zero(uint8_t *page_data)
{
    uint8_t i;
//  i = 0;
//  while ( (*page_data == 0) && (i < 16) )
//  {
//      i++;
//      page_data++;
//  }
//  if (i < 16) return 1;
//  else
//      return 0;

    for (i=0; i<16; i++)
    {
        if (*page_data != 0) return 1;
        page_data++;
    }
    return 0;
}

uint8_t one_if033::wb_cfg_prog_flash(uint8_t ufm, uint32_t prog_size, uint8_t *byte_data)
{
    return wb_cfg_prog_flash(ufm, prog_size, byte_data, 0);
}

uint8_t one_if033::wb_cfg_prog_flash(uint8_t ufm, uint32_t prog_size, uint8_t *byte_data, uint32_t ucode)
{
    uint32_t i;
    uint32_t status, npages;
    ufm &= 1;

    uint8_t prog_addr = 1;

    npages = prog_size >> 4;

    printf("Enable transparent configuration\n");
    wb_cfg_enable();
    if ( wb_wait_no_busy() )
        printf("*** Timeout waiting for busy -> 0\n");

    status = wb_cfg_get_status(0, 1);

    if ((status >> 13) & 1)
    {
        printf("*** Exit programming, as fail flag is 1\n");
        return 1;
    }

    if (prog_size == 0)
    {
        printf("Erase flash sector in ufm(1)/cfg(0) %d\n", ufm);

        wb_cfg_erase(ufm, 1-ufm, 0, 0);

        if ( wb_wait_no_busy() )
            printf("*** Timeout waiting for busy -> 0\n");

        status = wb_cfg_get_status(0, 1);

        if ((status >> 13) & 1)
        {
            printf("*** Exit programming, as fail flag is 1\n");
            wb_cfg_disable();
            return 1;
        }
        else
            printf("Device erased!\n");
    }

    if (prog_size > 0)
    {
        for (i=0; i < npages; i++)
        {
            if (page_non_zero(byte_data) )
            {
                if (prog_addr)
                {
                    set_address_fm(ufm, i);
                    prog_addr = 0;
                }
                wb_cfg_write_page(ufm, byte_data);
                status = wb_cfg_get_status(0, 0);
                if ((status >> 13) & 1)
                {
                    printf("*** Page %d, status read 0x%08x, fail flag is 1!!!\n",i, status);
                }
            }
            else
            {
                prog_addr = 1;
            }

            byte_data += 16;
        }
        if ((ufm == 0) && (ucode != 0) )
            prog_user_code_flash(ucode);
        printf("Send DONE\n");
        wb_cfg_set_done();
        status = wb_cfg_get_status(0, 1);
        if ((status >> 13) & 1)
        {
            printf("*** after DONE, status read 0x%08x, fail flag is 1!!!\n", status);
        }
        if (((status >> 8) & 1)==0)
        {
            printf("*** after DONE, status read 0x%08x, flag DONE is 0!!!\n", status);
        }
    }

//    printf("Disable transparent configuration\n");
    wb_cfg_disable();

    if ( (prog_size > 0) && (ufm == 0) )
    {
        printf("Refresh the config from the flash\n");
        wb_cfg_refresh();
        sleep(1);
        status = wb_cfg_get_status(0, 1);
        if ( ( (status >> 23) & 0x7) == 0)
            printf("Refresh was successfull!!!\n");
        else
            printf("Refresh was NOT successfull!!!\n");
    }

    return 0;
}

uint8_t one_if033::wb_cfg_flash_rd(uint8_t ufm, uint32_t data_size, uint8_t *byte_data)
{
    uint32_t npages, nbytes;
    uint32_t status;
    uint8_t cmd_code;

    ufm &= 1;
    if (ufm) cmd_code = 0xCA;
    else     cmd_code = 0x73;

    printf("Enable transparent configuration\n");
    wb_cfg_enable();
    if ( wb_wait_no_busy() )
        printf("Timeout waiting for busy -> 0\n");

    status = wb_cfg_get_status(0, 0);

    if ((status >> 13) & 1)
    {
        printf("Exit programming, as fail flag is 1\n");
        return 1;
    }

    set_address_fm(ufm, 0);
    npages = data_size >> 4; // divided by 16
    npages &= 0x3FFF;
    nbytes = npages*16;

    if (npages > 1) npages++; // read 1 dummy page at the beginning when command 0x73 10 0p pp

    wb_cnf_read(cmd_code, npages | (1 << 20), nbytes, byte_data);

    printf("Disable transparent configuration\n");
    wb_cfg_disable();
    return 0;
}

uint8_t one_if033::wb_cfg_flash_prd(uint8_t ufm, uint32_t data_size, uint8_t *byte_data)
{
    uint32_t npages, i, j;
    uint32_t status;
    uint8_t cmd_code;

    ufm &= 1;
    if (ufm) cmd_code = 0xCA;
    else     cmd_code = 0x73;

    printf("Enable transparent configuration\n");
    wb_cfg_enable();
    if ( wb_wait_no_busy() )
        printf("Timeout waiting for busy -> 0\n");

    status = wb_cfg_get_status(0, 0);

    if ((status >> 13) & 1)
    {
        printf("Exit programming, as fail flag is 1\n");
        return 1;
    }

    set_address_fm(ufm, 0);
    npages = data_size >> 4; // divided by 16
    npages &= 0x3FFF;

// reading page by page, takes longer, the result is the same
    printf("Starting read %d pages from ufm %d\n", npages, ufm);
    for (i=0; i<npages; i++)
    {
        wb_cnf_read(cmd_code, 0x100001, 16, byte_data);

        printf("Page 0x%04x: ",i);
        for (j=0; j<16; j++) printf(" 0x%02x",byte_data[j]);
        printf("\n");
        byte_data += 16;
    }

    printf("Disable transparent configuration\n");
    wb_cfg_disable();
    return 0;
}

uint32_t one_if033::read_user_code_flash()
{
    uint32_t status;

    wb_cfg_enable();
    status = read_user_code_sram();
    wb_cfg_disable();
    return status;
}

uint16_t one_if033::read_feat_bits()
{
    uint32_t status;

    wb_cfg_enable();
    status = wb_cnf_read(0xFB, 0x000000, 2);
    wb_cfg_disable();
    return status;
}

uint8_t one_if033::print_feat_bits(FILE *f, uint16_t fb)
{
    uint8_t bb, mspi, i2c, sspi, jtag, done, initn, programn, assp;
    fb >>= 4;
    assp = fb & 1;

    fb >>= 1;
    programn = fb & 1;

    fb >>= 1;
    initn = fb & 1;

    fb >>= 1;
    done = fb & 1;

    fb >>= 1;
    jtag = fb & 1;

    fb >>= 1;
    sspi = fb & 1;

    fb >>= 1;
    i2c = fb & 1;

    fb >>= 1;
    mspi = fb & 1;

    fb >>= 1;
    bb = fb & 3;

    fprintf(f, "# Feature bits: bb=%d, mspi=%d, i2c=%d, sspi=%d, jtag=%d, done=%d, initn=%d, programn=%d, my_assp=%d\n",
            bb, mspi, i2c, sspi, jtag, done, initn, programn, assp);
    fprintf(f, "# Special pins/ports enabled:");
    if (programn==0) fprintf(f, " PROGRAMN");
    if (initn   ==1) fprintf(f, " INITN");
    if (done    ==1) fprintf(f, " DONE");
    if (jtag    ==0) fprintf(f, " JTAG");
    if (sspi    ==0) fprintf(f, " SSPI");
    if (i2c     ==0) fprintf(f, " I2C");
    if (mspi    ==1) fprintf(f, " MSPI");
    fprintf(f, "\n");
    fprintf(f, "# Boot sequence: ");
    if ((bb==0) && (mspi==0)) fprintf(f, "Single boot from CFG flash\n");
    if ((bb==0) && (mspi==1)) fprintf(f, "Dual boot from CFG flash and external if there is a failure\n");
    if ((bb==1) && (mspi==1)) fprintf(f, "Single boot from external flash\n");

    return 0;
}

uint32_t one_if033::write_conf_file(FILE *f, uint8_t *prog_data, uint32_t length)
{
    uint32_t i;
    uint8_t *prog_data_end;

    prog_data_end = prog_data + length - 1; // point to the last
    while ( (*prog_data_end == 0) && (length > 0) )
    {
        length--;
        prog_data_end--;
    }

    for (i=0; i<length; i++)
        fprintf(f, "0x%02x\n",*prog_data++);
//              //fputc(cfg_data[i], f_out);
    return length;
}

uint32_t one_if033::read_conf_file(FILE *f, uint8_t *prog_data, uint32_t max_length)
{
    char linebuf[128];
    uint32_t dat;
    int  args;
    uint32_t nbytes;

    nbytes = 0;
    while ( (fgets(linebuf, 128, f)) && (nbytes < max_length) )
    {
        args = sscanf(linebuf, "%x", &dat);
        if (args >= 1)
        {
            *prog_data = dat & 0xFF;
            prog_data++;
            nbytes++;
        }
    }
    return nbytes;
}

uint32_t one_if033::read_imem_file(FILE *f, uint8_t *prog_data, uint32_t max_length)
{
    char linebuf[128];
    uint32_t dat;
    int  args;
    uint32_t nwords;

    nwords = 0;
    while ( (fgets(linebuf, 128, f)) && (nwords < max_length) )
    {
        args = sscanf(linebuf, "%x", &dat);
        if (args >= 1)
        {
            *prog_data = dat & 0xFF;
            prog_data++;
            *prog_data = (dat >> 8) & 0xFF;
            prog_data++;
            nwords++;
        }
    }
    return nwords;
}

int32_t one_if033::display_write(uint8_t reg0ram1, uint8_t wdata)
{
    reg0ram1 &= 1;
    if (display_debug) printf("Display: WRITE RS=%d, data 0x%02x\n", reg0ram1, wdata);
    return m_uart_crc->WriteByte(slave_addr, ADDR_DIS | reg0ram1 | AUTOINC_ADDR, wdata, false);
}

int32_t one_if033::display_read(uint8_t reg0ram1)
{
    int32_t err;
    reg0ram1 &= 1;
    err = m_uart_crc->ReadByte(slave_addr, ADDR_DIS | reg0ram1 | AUTOINC_ADDR, false);
    if (display_debug) printf("Display: READ  RS=%d, data 0x%02x\n", reg0ram1, err & 0xFF);
    return err;
}

int32_t one_if033::display_clear()
{
    return display_write(0, 1);
}

int32_t one_if033::display_ret_home()
{
    return display_write(0, 2);
}

int32_t one_if033::display_entry_set(uint8_t incr, uint8_t shift)
{
    incr  &= 1;
    shift &= 1;
    return display_write(0, 0x04 | (incr << 1) | shift);
}

int32_t one_if033::display_on_off(uint8_t dis_on, uint8_t curs_on, uint8_t blink_on)
{
    dis_on   &= 1;
    curs_on  &= 1;
    blink_on &= 1;
    return display_write(0, 0x08 | (dis_on << 2) | (curs_on << 1) | blink_on);
}

int32_t one_if033::display_curs_dis_shift(uint8_t curs0_dis1, uint8_t left0_right1)
{
    curs0_dis1   &= 1;
    left0_right1 &= 1;
    return display_write(0, 0x10 | (curs0_dis1 << 3) | (left0_right1 << 2));
}

int32_t one_if033::display_func_set(uint8_t byte1_nibble0, uint8_t L1_H2_lines, uint8_t L5x8_H5x11)
{
    byte1_nibble0 &= 1;
    L1_H2_lines   &= 1;
    L5x8_H5x11    &= 1;
    return display_write(0, 0x20 | (byte1_nibble0 << 4) | (L1_H2_lines << 3) | (L5x8_H5x11 << 2));
}

int32_t one_if033::display_set_cg_addr(uint8_t addr)
{
    addr  &= 0x3F;
    return display_write(0, 0x40 | addr);
}

int32_t one_if033::display_set_dr_addr(uint8_t addr)
{
    addr  &= 0x7F;
    return display_write(0, 0x80 | addr);
}

int32_t one_if033::display_wr_data(uint8_t wdata)
{
    return display_write(1, wdata);
}

int32_t one_if033::display_rd_data()
{
    return display_read(1);
}

int32_t one_if033::display_rd_baddr()
{
    return display_read(0);
}

int32_t one_if033::display_wait_nbusy(uint16_t max_tries)
{
    uint8_t b;
    uint16_t i;

    for (i=0; i<max_tries; i++)
    {
        if ( (display_rd_baddr() & 0x80) == 0) return i;
    }
    return -1;
}

