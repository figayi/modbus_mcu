/*
 * File: modbus.h
 * Project: modbus
 * File Created: May 2nd 2018 9:07:21 am
 * Author: zhaohy
 * -----
 * Last Modified: June 7th 2018 11:45:02 am
 * Modified By: zhaohy
 * -----
 * Copyright (c) 2017 - 2018 LKcore Co., Ltd.
 */

#ifndef MODBUS_H
#define MODBUS_H

#include "mcudef.h"
/*Modbus function codes*/
/* clang-format off */
#define MODBUS_FC_READ_COILS                            0x01
#define MODBUS_FC_READ_DISCRETE_INPUTS                  0x02
#define MODBUS_FC_READ_HOLDING_REGISTERS                0x03
#define MODBUS_FC_READ_INPUT_REGISTERS                  0x04
#define MODBUS_FC_WRITE_SINGLE_COIL                     0x05
#define MODBUS_FC_WRITE_SINGLE_REGISTER                 0x06
#define MODBUS_FC_READ_EXCEPTION_STATUS                 0x07
#define MODBUS_FC_WRITE_MULTIPLE_COILS                  0x0F
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS              0x10
#define MODBUS_FC_REPORT_SLAVE_ID                       0x11
#define MODBUS_FC_MASK_WRITE_REGISTER                   0x16
#define MODBUS_FC_WRITE_AND_READ_REGISTERS              0x17

#define MODBUS_BROADCAST_ADDRESS                        0
#define MODBUS_MAX_ADU_LENGTH                           256
#define MDOBUS_MIN_ADU_LENGTH                           4
#define MODBUS_EXCEPTION_LENGTH                         3
#define MODBUS_RTU_CHECKSUM_LENGTH                      2
#define INPUT_REGISTERS_NB                              32
#define HOLDING_REGISTERS_NB                            32
/* clang-format on */
enum {
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 0x01,
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS,
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE,
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE,
    MODBUS_EXCEPTION_ACKNOWLEDGE,
    MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY,
    MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE,
    MODBUS_EXCEPTION_MEMORY_PARITY,
    MODBUS_EXCEPTION_NOT_DEFINED,
    MODBUS_EXCEPTION_GATEWAY_PATH,
    MODBUS_EXCEPTION_GATEWAY_TARGET,
    MODBUS_EXCEPTION_MAX
};

typedef enum {
    MODBUS_IDLE = 0,
    MODBUS_RECEIVING,
    MODBUS_WAIT_RESPONSE,
} modbus_state_t;

typedef struct {
    uint8_t rxbuf[MODBUS_MAX_ADU_LENGTH];
    uint16_t rxlen;
    int state;
    int timeout;
} modbus_t;

typedef struct {
    uint16_t input_registers[INPUT_REGISTERS_NB];
    uint16_t holding_registers[HOLDING_REGISTERS_NB];
} modbus_mapping_t;

/**
 * @brief 检验接收数据完整性，会校验slave和数据长度以及CRC
 *
 * @param req   modbu_t类型
 * @param slave   从机编号
 * @return  int 数据长度
 */
int modbus_check_integrity(modbus_t *req, int slave);

/**
 * @brief 解析request并返回数据
 *
 * @param request 请求数据
 * @param modbus_mapping_t  寄存器映射区
 * @return  int 数据长度
 */
int modbus_response(modbus_t *request, modbus_mapping_t *mb_mapping);

#endif