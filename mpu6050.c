#include "mpu6050.h"

UINT8 mpu_address;

void setupMPU6050(I2C_MODULE i2c, BOOL ad0) {
    /*
    if(i2c == I2C1) {
        OpenI2C1(I2C_CONFIG, 0xC2);
    } else if(i2c == I2C2) {
        OpenI2C2(I2C_CONFIG, 0xC2);
    }
    */
    initI2C();
    printf("Setting up MPU6060\n\r");
    mpu_address = (UINT8)MPU6050_ADDRESS_AD0_LOW;

    if(mpu_address == MPU6050_ADDRESS_AD0_LOW) {
        printf("MPU6050 AD0: low\n\r");
    } else if(mpu_address = MPU6050_ADDRESS_AD0_HIGH) {
        printf("MPU6050 AD0: high\n\r");
    }

    printf("Disabling sleep\n\r");
    requestWriteByte((UINT8)MPU6050_ADDRESS_AD0_LOW, (UINT8)MPU6050_RA_PWR_MGMT_1, 0x01);
    DelayMs(100);
    printf("Setup OK\n\r");
}



void DelayMs(unsigned int msec)
 {
     unsigned int tWait, tStart;
     tWait=(80000000ul/2000)*msec;        //    SYS_FREQ        (80000000)
     tStart=ReadCoreTimer();
     while((ReadCoreTimer()-tStart)<tWait);        // wait for the time to pass.
 }