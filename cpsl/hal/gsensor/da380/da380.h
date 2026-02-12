

#ifndef _DA380_H
#define _DA380_H

#define WRITE_COMMAND_FOR_DA380       0x4e
#define READ_COMMAND_FOR_DA380        0x4f

//Read Only
#define RESET_DA380         0x00
#define CHIPID_DA380        0x01
#define ACC_X_LSB           0x02
#define ACC_X_MSB           0x03
#define ACC_Y_LSB           0x04
#define ACC_Y_MSB           0x05
#define ACC_Z_LSB           0x06
#define ACC_Z_MSB           0x07
#define MOTION_FLAG         0x09
#define NEWDATA_FLAG        0x0A
#define TAP_ACTIVE_STATUS   0x0B//Read Only
#define ORIENT_STATUS       0x0C

//R/W
#define RESOLUTION_RANGE    0x0F//
#define ODR_AXIS            0x10
#define MODE_BW             0x11
#define SWAP_POLARITY       0x12
#define INT_SET1            0x16//R/W
#define INT_SET2            0x17
#define INT_MAP1            0x19
#define INT_MAP2            0x1A
#define INT_CONFIG          0x20
#define INT_LTACH           0x21
#define FREEFALL_DUR        0x22
#define FREEFALL_THS        0x23
#define FREEFALL_HYST       0x24
#define ACTIVE_DUR          0x27
#define ACTIVE_THS          0x28
#define TAP_DUR             0x2A//
#define TAP_THS             0x2B//
#define ORIENT_HYST         0x2C
#define Z_BLOCK             0x2D

#define SELF_TEST           0x32
#define CUSTOM_OFF_X        0x38
#define CUSTOM_OFF_Y        0x39
#define CUSTOM_OFF_Z        0x3A
#define CUSTOM_FLAG         0x4E
#define CUSTOM_CODE         0x4F
#define Z_ROT_HODE_TM       0x51
#define Z_ROT_DUR           0x52
#define ROT_TH_H            0x53
#define ROT_TH_L            0x54

enum G_SENSITY_LEVEL{
    G_SENSITY_HIGH = 0,
    G_SENSITY_MEDIUM,
    G_SENSITY_LOW
};

enum G_SENSITY_MODE{
    G_NORMAL_MODE = 0,
    G_LOW_POWER_MODE,
    G_SUSPEND_MODE
};


#endif


