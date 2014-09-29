/* 
 * File:   main.c
 * Author: tesla
 *
 * Created on September 25, 2014, 12:31 PM
 */

#include <plib.h>
#include "mpu6050.h"
#include "config_bits.h"

#define MPU6050_I2C1

void initUART(void);
void initAll(void);

/*
 * 
 */

int main(void) {
    UINT8 mpu_address = MPU6050_ADDRESS_AD0_LOW;
    initAll();
    UINT8 acc[2] = {0};
    INT16 acc16 = 0;
    float acc_g = 0;
    UINT8 data = 0;
    while(TRUE) {
        requestReadBytes(MPU6050_ADDRESS_AD0_LOW, (UINT8)MPU6050_RA_ACCEL_XOUT_H, acc, 2); // burst read two bytes.
        PORTToggleBits(IOPORT_F, BIT_0); // LED5.
        DelayMs(1000);
        acc16 = (INT16)((acc[0] << 8) | acc[1]);
        acc_g = (float)acc16 / 16384.0;
        printf("\n\rACC X: %f\n\r", acc_g);
    }
    return (EXIT_SUCCESS);
}

void initUART(void) {
    __XC_UART = 1;
    OpenUART1( UART_EN | UART_NO_PAR_8BIT | UART_1STOPBIT | UART_BRGH_SIXTEEN,
               UART_RX_ENABLE | UART_TX_ENABLE,
               GetPeripheralClock()/16/BAUD_RATE - 1);
    while( BusyUART1()); // Wait until the UART module is free.
    putsUART1("Initializing UART1...\n\r");
}

void initAll(void) {
    initUART();
    setupMPU6050(I2C1, FALSE); // I2C bus, and AD0 logic state.
    mPORTFSetPinsDigitalOut(BIT_0); // LED5.
    mPORTGSetPinsDigitalOut(BIT_6); // LED4.
}