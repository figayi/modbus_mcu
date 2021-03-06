/*
 * File: modbus.c
 * Project: modbus
 * File Created: June 7th 2018 7:01:39 pm
 * Author: yim
 * -----
 * Last Modified: June 16th 2018 4:49:20 am
 * Modified By: yim
 * -----
 * Copyright (c) 2017 - 2018 LKcore Co., Ltd.
 */


#include "modbus.h"
#include "string.h"
#include "uart.h"

uint16_t input_registers[10];
uint16_t holding_registers[10];
/* Table of CRC values for high-order byte */
static const uint8_t table_crc_hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80,
    0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
    0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80,
    0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80,
    0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40};

/* Table of CRC values for low-order byte */
static const uint8_t table_crc_lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D,
    0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB,
    0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2,
    0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37,
    0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8,
    0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24,
    0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63,
    0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,
    0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F,
    0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C,
    0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46,
    0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40};

/*
    查表法计算CRC16，内部已将高位字节和低位字节交换
*/
static uint16_t crc16(const uint8_t *buffer, uint16_t buffer_length) {
    uint8_t crc_hi = 0xFF; /* high CRC byte initialized */
    uint8_t crc_lo = 0xFF; /* low CRC byte initialized */
    unsigned char i;       /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--) {
        i = crc_hi ^ *buffer++; /* calculate the CRC  */
        crc_hi = crc_lo ^ table_crc_hi[i];
        crc_lo = table_crc_lo[i];
    }

    return (crc_hi << 8 | crc_lo);
}

/*
    计算生成CRC16，仅作为生成测试数据使用
*/
static uint16_t cal_crc16(const unsigned char *buffer, uint16_t buffer_length) {
    uint16_t crc = 0xFFFF; /*CRC byte initialized */
    while (buffer_length--) {
        crc ^= *buffer++;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x01)
                crc = crc >> 1 ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    crc = ((crc & 0xff) << 8) + (crc >> 8);
    return crc;
}

/*
    初始化modbus
*/
int modbus_init(int serial, int baudrate) {
    return 1;
}

/*
    发送返回数据
*/
static int modbus_send_msg(uint8_t *msg, int length) {
    for (size_t i = 0; i < length; i++) {
        UART0_Write_Byte(*msg++);
    }
    return length;
}
/*
    检验接收数据完整性
*/
int modbus_check_integrity(modbus_t *req, int slave) {
    uint16_t crc_calculated;
    uint16_t crc_received;
    uint8_t *msg = req->rxbuf;
    int msg_length = req->rxlen;
    /* Filter on the Modbus unit identifier (slave) in RTU mode to avoid useless
     * CRC computing. */
    if (slave != msg[0] && slave != MODBUS_BROADCAST_ADDRESS) {
        //printf("Request for slave %d ignored (not %d)\n", slave, msg[0]);
        /* Following call to check_confirmation handles this error */
        return -1;
    }

    if (req->rxlen < MDOBUS_MIN_ADU_LENGTH || req->rxlen > MODBUS_MAX_ADU_LENGTH) {
        //printf("Request Error. The length should be between 4 and 256\n");
        return -1;
    }

    crc_calculated = crc16(msg, msg_length - 2);
    crc_received = (msg[msg_length - 2] << 8) | msg[msg_length - 1];

    /* Check CRC of msg */
    if (crc_calculated == crc_received) {
        return msg_length;
    } else {
        //printf("CRC error!\n");
        return -1;
    }
}

/*
    查表法计算CRC16，内部已将高位字节和低位字节交换
*/
static int response_exception(int slave, int function, int exception_code, uint8_t *rsp) {
    /* Build exception response */
    rsp[0] = slave;
    rsp[1] = function + 0x80;
    rsp[2] = exception_code;

    return MODBUS_EXCEPTION_LENGTH;
}

int modbus_response(modbus_t *request, modbus_mapping_t *mb_mapping) {
    uint8_t *req = request->rxbuf;
    uint8_t rsp[256];
    int rsp_length = 0;
    uint8_t slave = req[0];
    uint8_t function = req[1];
    uint16_t address = (req[2] << 8) + req[3];

    switch (function) {
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_READ_INPUT_REGISTERS: {
            uint16_t *registers = (function == MODBUS_FC_READ_INPUT_REGISTERS) ? mb_mapping->input_registers
                                                                               : mb_mapping->holding_registers;
            int nb = (req[4] << 8) + req[5];
            rsp[0] = slave;
            rsp[1] = function;
            rsp[2] = nb * 2;
            rsp_length = 3;
            for (int i = address; i < address + nb; ++i) {
                rsp[rsp_length++] = registers[i] >> 8;
                rsp[rsp_length++] = registers[i] & 0xFF;
            }
        } break;
        case MODBUS_FC_WRITE_SINGLE_REGISTER: {
            mb_mapping->holding_registers[address] = (req[4] << 8) + req[5];
            rsp_length = request->rxlen - MODBUS_RTU_CHECKSUM_LENGTH;
            memcpy(rsp, req, rsp_length);
        } break;
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS: {
            int nb = (req[4] << 8) + req[5];
            int i, j;
            for (i = address, j = 7; i < address + nb; i++, j += 2) {
                mb_mapping->holding_registers[i] = (req[j] << 8) + req[j + 1];
            }
            rsp_length = 6;
            memcpy(rsp, req, rsp_length);
        } break;
        default:
            //printf("Unknown Modbus function code: 0x%X\n", function);
            rsp_length = response_exception(slave, function, MODBUS_EXCEPTION_ILLEGAL_FUNCTION, rsp);
            break;
    }
    uint16_t crc = crc16(rsp, rsp_length);
    rsp[rsp_length++] = crc >> 8;
    rsp[rsp_length++] = crc & 0xFF;
    return (slave == MODBUS_BROADCAST_ADDRESS) ? 0 : modbus_send_msg(rsp, rsp_length);
}
