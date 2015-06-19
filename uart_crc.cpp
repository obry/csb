#include "iostream"
#include "ctime"
#include "crc8.h"
#include "uart.h"
#include "uart_crc.h"
//#include "uart_classes.h"



// $Id: uart_classes.cpp 41 2015-04-22 13:09:52Z angelov $:

//#define EMULATION

/* uart_crc CLASS */

uart_crc::uart_crc(uart *uart_used)
{
    uart_phys = uart_used;
    debug = 0;
}

uart_crc::~uart_crc()
{
}


bool uart_crc::SendBurst(uint8_t slaveID, uint16_t psize, uint16_t saddr, uint8_t *Bytes, bool ignore_crc)
{
    int i;
    crc8 crc_reg;
    uint8_t wrk;
    uint8_t my_bytes[8], j;
    uint16_t rep_cnt = 0;
    uint16_t err;
    uint16_t blk_size;

    blk_size = psize;
    if (blk_size > 2048)
        printf("Invalid block size!!!\n");
    if (debug) printf("SendBurst, slave %d, packet size %d, address in slave 0x%04x\n",
                        slaveID, psize, saddr);
    do
    {
        err = 0;
        j = 0;

        my_bytes[j] = (((psize-1) & 0x7) << 5) | (slaveID << 1);
        if (debug) printf("Sending the slave ID...\n");
        crc_reg.addbyte(my_bytes[j++]);

        my_bytes[j] = ( (psize-1) >> 3) & 0xFF;
        if (debug) printf("Sending the upper part of the packet size...\n");
        crc_reg.addbyte(my_bytes[j++]);

        if (debug) printf("Sending the internal address...\n");

        my_bytes[j] = saddr & 0xFF;
        crc_reg.addbyte(my_bytes[j++]);

        my_bytes[j] = saddr >> 8;
        crc_reg.addbyte(my_bytes[j++]);
        uart_phys->WriteBuffer(my_bytes, j);

        if (debug) printf("sending the %d-bytes data...\n", blk_size);
        uart_phys->WriteBuffer(Bytes, blk_size);

        for (i=0; i<blk_size; i++)
            crc_reg.addbyte(Bytes[i]);

        if (debug) printf("sending the crc...\n");
        uart_phys->WriteByte(crc_reg.get_crc()); // send checksum here

        wrk = uart_phys->ReadByte();
        if (ignore_crc == false)
        {
            if (wrk != 0)
            {
                printf("SendBurst:Sent %d bytes, received bad CRC8 0x%02x, slave=%d, addr=0x%02x\n", blk_size, wrk, slaveID, saddr);
                err++;
            }
            rep_cnt++;
            if (err > 0)
            {
                printf(" Sending break %d\n", rep_cnt);
                uart_phys->SendBreak (1);
                do
                {
                    printf("SendBurst:clear buffer\n");
                }
                while (uart_phys->ReadByte() != EOF);
                printf("SendBurst:buffer cleared\n");
                crc_reg.init();
            }
        }
    }
    while ((rep_cnt < 2) && (err > 0));
    if (err > 0) return false;
    else         return true;
}

bool uart_crc::RecvBurst(uint8_t slaveID, uint16_t psize, uint16_t saddr, uint8_t *Bytes, bool ignore_crc)
{
    int i;
    uint8_t my_bytes[8], j;
    crc8 crc_reg;
    uint8_t wrk;
    uint16_t rep_cnt = 0;
    uint16_t err;
    uint16_t blk_size;

    blk_size = psize;
    if (blk_size > 2048)
        printf("Invalid block size!!!\n");

    if (debug) printf("RecvBurst, slave %d, packet size %d, address in slave 0x%04x\n",
                        slaveID, psize, saddr);
    do
    {
        err = 0;

        j = 0;

        my_bytes[j] = (((psize-1) & 0x7) << 5) | (slaveID << 1) | 1;
        if (debug) printf("Sending the slave ID...\n");
        crc_reg.addbyte(my_bytes[j++]);

        my_bytes[j] = ( (psize-1) >> 3) & 0xFF;
        if (debug) printf("Sending the upper part of the packet size...\n");
        crc_reg.addbyte(my_bytes[j++]);

        if (debug) printf("Sending the internal address...\n");

        my_bytes[j] = saddr & 0xFF;
        crc_reg.addbyte(my_bytes[j++]);

        my_bytes[j] = saddr >> 8;
        crc_reg.addbyte(my_bytes[j++]);

        my_bytes[j++] = crc_reg.get_crc();

        uart_phys->WriteBuffer(my_bytes, j);

        crc_reg.init();
        // receive CRC8
        wrk = uart_phys->ReadByte();
        crc_reg.addbyte(wrk); // all sent data including the sent crc must be counted!!!
        if (wrk != 0)
        {
            printf("RecvBurst:Sent header, received bad CRC8 0x%02x, slave=%d, addr=0x%02x, repcnt=%d\n", wrk, slaveID, saddr, rep_cnt);
            if (ignore_crc == false) err++;
        }
        // receive data
        uart_phys->ReadBuffer(Bytes, blk_size);

        for (i=0; i<blk_size; i++)
        {
//            Bytes[i] = uart_phys->ReadByte();
            crc_reg.addbyte(Bytes[i]);
        }

        // receive CRC8
        wrk = uart_phys->ReadByte();
        crc_reg.addbyte(wrk);
        if ( (crc_reg.get_crc() != 0) && (ignore_crc == false) )
        {
            printf("RecvBurst:Received %d bytes with bad CRC8 0x%02x, slave=%d, addr=0x%02x, repcnt=%d\n", blk_size, wrk,
                                    slaveID, saddr, rep_cnt);
            if (ignore_crc == false) err++;
        }
        rep_cnt++;
        if (err > 0)
        {
            printf(" Sending break %d\n", rep_cnt);
            uart_phys->SendBreak (1);
            do
            {
                printf("RecvBurst:clear buffer\n");
            }
            while (uart_phys->ReadByte() != EOF);
            printf("RecvBurst:buffer cleared\n");
            crc_reg.init();
        }
    }
    while ((rep_cnt < 2) && (err > 0));
    if (err > 0) return false;
    else         return true;
}

uint16_t uart_crc::ReadWord(uint8_t slaveID, uint16_t saddr, bool ignore_crc)
{
    uint8_t mybytes[2];
    uint16_t res;
    RecvBurst(slaveID, 2, saddr, mybytes, ignore_crc);
    res = mybytes[1];
    res = (res << 8) | mybytes[0];
    return res;
}

uint32_t uart_crc::Read3Bytes(uint8_t slaveID, uint16_t saddr, bool ignore_crc)
{
    uint8_t mybytes[4];
    uint32_t res;
    RecvBurst(slaveID, 3, saddr, mybytes, ignore_crc);
    res = mybytes[2];
    res = (res << 8) | mybytes[1];
    res = (res << 8) | mybytes[0];
    return res;
}

uint8_t uart_crc::ReadByte(uint8_t slaveID, uint16_t saddr, bool ignore_crc)
{
    uint8_t mybyte;
    RecvBurst(slaveID, 1, saddr, &mybyte, ignore_crc);
    return mybyte;
}

bool uart_crc::WriteByte(uint8_t slaveID, uint16_t saddr, uint8_t wdata, bool ignore_crc)
{
    return SendBurst(slaveID, 1, saddr, &wdata, ignore_crc);
}

bool uart_crc::WriteWord(uint8_t slaveID, uint16_t saddr, uint16_t wdata, bool ignore_crc)
{
    uint8_t mybytes[2];
    mybytes[0] = wdata & 0xFF;
    mybytes[1] = wdata >> 8;
    if (debug)
        printf("write word, slave id=%d, internal address=0x%02x, data=0x%04x\n",slaveID, saddr, wdata);
    return SendBurst(slaveID, 2, saddr, mybytes, ignore_crc);
}

/* END OF uart_crc CLASS */

