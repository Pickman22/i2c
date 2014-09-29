#ifndef I2C_H
#define I2C_H

#include <plib.h>
#include <GenericTypeDefs.h>
#include "clock.h"
#include "utils.h"

#define USE_I2C1 // Determines I2C bus to be used.
#define I2C_RATE 100000 // 100khz transfer rate.
#define TPGD 0.000000004 // 4ns
#define MAX_BUFFER_LEN 255 // 256 bytes.

typedef enum error_t {
    NO_ERROR = 0,
    START_ERROR = 2,
    BUS_COLLISION_ERROR = 4,
    ACKNOWLEDGE_ERROR = 8,
} error;

typedef enum I2CStates_t {
	send_header = 1,
	repeat_start,
	read_command,
	prepare_read,
	start_writing,
	start_reading,
	stop,
	wait_done,
	done
}I2CStates;

typedef union I2CEvent_t{
    UINT8 masterEvent: 1;
    UINT8 busCollision: 1;
}I2CEvent;

typedef struct i2cbuffer_t {
	UINT8 header[2];
	UINT8 wdata[MAX_BUFFER_LEN];
	UINT8* rdata;
	UINT8 len;
	UINT8 index;
    BOOL mode;
    BOOL ready;
}I2CBuffer;

#define READ_MODE 1
#define WRITE_MODE 0

void setupI2CInterrupts(void);
void initI2C(void);

error startTransfer(BOOL repeat);
error stopTransfer(void);
error writeByte(UINT8 data);
error writeBytes(UINT8 *arr, UINT8 len);
error readByte(UINT8 *data, BOOL ack);
error readBytes(UINT8 *data, UINT8 len);

error singleRead(UINT8 device_address, UINT8 reg_address, UINT8 *data);
error burstRead(UINT8 device_address, UINT8 reg_address, UINT8 *data, UINT8 len);
void debug(error err);


/************************************************************************************************
• Start Condition – 1 BRG time after falling edge of SDAx
• Repeated Start Sequence – 1 BRG time after falling edge of SDAx
• Stop Condition – 1 BRG time after the rising edge of SDAx
• Data transfer byte received – Eighth falling edge of SCLx (after receiving eight bits of data
  from slave)
• During a Send ACK sequence – Ninth falling edge of SCLx (after sending ACK or NACK to
  slave)
• Data transfer byte transmitted – Ninth falling edge of SCLx (regardless of receiving ACK
  from slave)
• During a slave-detected Stop – When slave sets the P bit (I2CxSTAT<4>)
*************************************************************************************************/

/* Event driven I2C protocol */

void requestStartTransfer(BOOL repeat);
//void requestStopTransfer(void);
void requestWriteByte(UINT8 slave_address, UINT8 reg_address, UINT8 data);
void requestWriteBytes(UINT8 slave_address, UINT8 reg_address, UINT8* dat_arr, UINT8 len); 
void requestReadByte(UINT8 slave_address, UINT8 reg_address, UINT8* data); 
void requestReadBytes(UINT8 slave_address, UINT8 reg_address, UINT8* dat_arr, UINT8 len);

#define I2C_CONFIG I2C_EN | I2C_SLW_DIS | I2C_IDLE_CON | I2C_7BIT_ADD | I2C_GC_DIS | I2C_SM_DIS


//#define I2C_BRG 0xC2

#ifdef USE_I2C1
#define I2CCONbits I2C1CONbits
#define I2CSTATbits I2C1STATbits
#define I2CCON I2C1CON
#define I2CSTAT I2C1STAT
#define I2CTRN I2C1TRN
#define I2CRCV I2C1RCV

#define _I2C_VECTOR _I2C_1_VECTOR
#define INT_I2CB INT_I2C1B
#define INT_I2CM INT_I2C1M
#define INT_I2C INT_I2C1
#define INT_I2C_VECTOR INT_I2C_1_VECTOR
#define OpenI2C OpenI2C1
#endif

#ifdef USE_I2C2
#define I2CCONbits I2C2CONbits
#define I2CSTATbits I2C2STATbits
#define I2CCON I2C2CON
#define I2CSTAT I2C2STAT
#define I2CTRN I2C2TRN
#define I2CRCV I2C2RCV

#define _I2C_VECTOR _I2C_2_VECTOR
#define INT_I2CB INT_I2C2B
#define INT_I2CM INT_I2C2M
#define INT_I2C INT_I2C2
#define INT_I2C_VECTOR INT_I2C_2_VECTOR
#define OpenI2C OpenI2C2
#endif


#endif // I2C_H