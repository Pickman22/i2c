#include "i2c.h"

I2CEvent events;
static I2CBuffer buffer;
static I2CStates state = done;

void initI2C(void) {
    events.busCollision = FALSE;
    events.masterEvent = FALSE;
    unsigned int brg = 0;
    unsigned int plck = (unsigned int)GetPeripheralClock();
    unsigned int i2c_clk = (unsigned int)I2C_RATE;
    unsigned int fpgd = (unsigned int)1/TPGD;
    brg = plck / (2 * i2c_clk);
    float dummy = plck / fpgd;
    brg -= (unsigned int)dummy + 2;

    OpenI2C(I2C_CONFIG, brg);
    setupI2CInterrupts();
}

void setupI2CInterrupts(void) {
    INTEnableSystemMultiVectoredInt();
    INTSetVectorPriority(INT_I2C_VECTOR, INT_PRIORITY_LEVEL_2);
    INTSetVectorSubPriority(INT_I2C_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTClearFlag(INT_I2C);
    INTEnable(INT_I2CM | INT_I2CB, INT_ENABLED);
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    INTEnableInterrupts();
}

error startTransfer(BOOL restart) {
    while(I2CCONbits.SEN || I2CCONbits.PEN || I2CCONbits.RSEN || I2CCONbits.RCEN 
        || I2CCONbits.ACKEN || I2CSTATbits.TRSTAT);
    if(restart) {
        I2CCONbits.RSEN = TRUE; // Set repeated start event.
        while(I2CCONbits.RSEN); // Wait for Repeated Start to finish
    } else {
        I2CCONbits.SEN = TRUE; // Start condition
        while(I2CCONbits.SEN);  // Wait for Start to finish.
        if(I2CSTATbits.BCL == TRUE) {
            return BUS_COLLISION_ERROR;
        }
    }
    return NO_ERROR;
}

error stopTransfer(void) {
    /*while(I2CCONbits.SEN || I2CCONbits.PEN || I2CCONbits.RSEN || I2CCONbits.RCEN 
        || I2CCONbits.ACKEN || I2CSTATbits.TRSTAT);*/
    I2CCONbits.PEN = TRUE; // Activate stop event.
    while(I2CCONbits.PEN); // Wait for stop to finish.
    return NO_ERROR;
}

error writeByte(UINT8 data) {
    while(I2CSTATbits.TRSTAT); // Wait for transmitter.
    I2C1TRN = data; // Load data.
    if(I2CSTATbits.IWCOL) { // Test for collision event.
        return BUS_COLLISION_ERROR;
    }
    while(I2CSTATbits.TRSTAT); // Wait for transmitter to finish sending data.
    if(I2CSTATbits.ACKSTAT) { // Test for acknowledge from slave.
        return ACKNOWLEDGE_ERROR;
    }
    return NO_ERROR;
}

error writeBytes(UINT8 *arr, UINT8 len) {
    UINT8 i;
    error err;
    for(i = 0; i < len; ++i) {
        err = writeByte(arr[i]);
        if(err != NO_ERROR) {
            return err;
        }
    }
    return NO_ERROR;
}

error readByte(UINT8 *data, BOOL ack) {
    I2CCONbits.ACKDT = !ack; // Configure ack or nack.
    I2CCONbits.RCEN = TRUE; // Enable receiver.
    while(!I2CSTATbits.RBF | I2CCONbits.RCEN); // Wait for Receive buffer.ull flag and receiver disabled.
    *data = I2C1RCV; // Read register data.
    //while(I2C1CON & (unsigned int)0x0000001F); // Bus idle.
    //while(I2CSTATbits.S);
    I2CCONbits.ACKEN = 1; // Send acknowledge.
    while(I2CCONbits.ACKEN); // Wait for ack to finish.
    return NO_ERROR;
}

error readBytes(UINT8 *data, UINT8 len) {
    int i;
    BOOL ack = TRUE;
    for(i = 0; i < len; ++i) {
        if((i+1) == len) {
            ack = FALSE; // NACK for last UINT8.
        }
        error err = readByte(data++, ack);
        if(err != NO_ERROR) return err;
    }
    return NO_ERROR;
}

error burstRead(UINT8 device_address7bit, UINT8 reg_address, UINT8 *data, UINT8 len) {
    UINT8 add_w, add_r;
    add_w = (device_address7bit << 1) & 0xFE; // Clear lsb to command write.
    add_r = (device_address7bit << 1) | 0x1; // Set lsb to command read.

    UINT8 msg[] = {add_w, reg_address}; // Point to register to be read
    error err = startTransfer(FALSE);
    err = writeBytes(msg, 2); // Address slave.
    if (err != NO_ERROR) {
        return err;
    }
    err = startTransfer(TRUE); // Repeat start sequence
    if (err != NO_ERROR) {
        return err;
    }
    writeByte(add_r); // Ask for data.
    err = readBytes(data, len); // Get "len" data bytes.
    if (err != NO_ERROR) return err;
    stopTransfer();
    return NO_ERROR;
}



error singleRead(UINT8 device_address7bit, UINT8 reg_address, UINT8 *data) {
    UINT8 add_w, add_r;
    add_w = (device_address7bit << 1) & 0xFE; // Clear lsb to command write.
    add_r = (device_address7bit << 1) | 0x1; // Set lsb to command read.
    error err;
    UINT8 msg[] = {add_w, reg_address}; // Point to register to be read.
    startTransfer(FALSE);
    err = writeBytes(msg, 2); // Address slave.
    err = startTransfer(TRUE); // Repeat start sequence
    err = writeByte(add_r); // Command read.
    err = readByte(data, FALSE); // Get data UINT8. Not acknowledge.
    stopTransfer();
    return NO_ERROR;
}

void debug(error err) {
    switch(err) {
        case START_ERROR:
            putsUART1("START_ERROR\n\r");
            break;
        case BUS_COLLISION_ERROR:
            putsUART1("BUS_COLLISION_ERROR\n\r");
            break;
        case ACKNOWLEDGE_ERROR:
            putsUART1("ACK_ERROR\n\r");
            break;
        case NO_ERROR:
            putsUART1("NO_ERROR\n\r");
            break;
        default:
            putsUART1("UNKNOWN_ERROR\n\r");
    }
}

void __ISR(_I2C_VECTOR, IPL2) _masterEventISR(void) {
    if(INTGetFlag(INT_I2CM)) {
        
        switch(state) {

            case send_header: // Send two header bytes to address the slave.
            putcUART1('1');
            //Keep sending the message till header is finished.
            if(buffer.index == 0) {
                putcUART1('2');
                buffer.index++; // Point to the next byte in header.
                I2CTRN = buffer.header[0]; // Write slave address.
            }
            else { // If the first byte is already send, prepare for next state.
                putcUART1('3');
                if(buffer.mode == READ_MODE) { // Read mode.
                    putcUART1('4');
                    state = repeat_start; // A repeat start event after writing last header byte.    
                } else { // Write mode.
                    putcUART1('5');
                    state = start_writing; // Go write register data after sending last header byte.
                }
                buffer.index = 0; // Restart index to begin reading or writing.
                putcUART1('6');
                if(I2CSTATbits.ACKSTAT) {
                    putcUART1('X'); // not acknowledge.
                }
                //printf("reset index\n\r");
            }            
            I2CTRN = buffer.header[1];
            break;

            case start_writing: // Send data to slave.
            if(I2CSTATbits.ACKSTAT) {
                    putcUART1('X'); // Last byte not acknowledged.
            }
            putcUART1('7');
            if((buffer.index+1) == buffer.len) { // Last byte.
                putcUART1('8');
                state = stop; // Stop after sending last byte.
                I2CTRN = buffer.wdata[buffer.index];  // Do not point to next byte as this is the last one.
            }
            // Write data and point to the next one. Keep in mind that when writing the last data byte,
            // index variable still increments one after writing.
            else {
                putcUART1('9');
                I2CTRN = buffer.wdata[buffer.index++]; // Write byte and point to the next one.
            }
            break;

            case repeat_start: // Repeat start to read.
            state = read_command; // Prepare to receive data.
            putcUART1('A');
            I2CCONbits.RSEN = TRUE; // Generate repeat start event.
            break;

            case read_command: // Generate a read command.
            putcUART1('a');
            state = prepare_read;
            I2CTRN = (buffer.header[0] | 0x1);
            break;

            case prepare_read: // Read command transmited. Enable receiver.
            if(I2CSTATbits.ACKSTAT) {
                    putcUART1('X'); // not acknowledge.
            }
            putcUART1('b');
            state = start_reading;
            I2CCONbits.RCEN = TRUE;
            break;

            case start_reading: // Read data from I2RCV register. And re-enable receiver if needed.
            putcUART1('B');
            if((buffer.index+1) == buffer.len) { // Last byte. No acknowledge.
                *(buffer.rdata + buffer.index) = I2CRCV; // Read data and finish.
                I2CCONbits.ACKDT = TRUE; // Configure nack.
                state = stop;
                putcUART1('C');
            } else {
                I2CCONbits.ACKDT = FALSE; // Configure ack. Keep reading.
                *(buffer.rdata + buffer.index++) = I2CRCV; // Read register data. Point to next slot.
                I2CCONbits.RCEN = TRUE; // Prepare receiver.
                putcUART1('D');
            }      
            I2CCONbits.ACKEN = 1; // Acknowledge generation.
            break;

            case stop: // Stop event.
            putcUART1('E');
            state = wait_done; // Nothing lef to do.
            I2CCONbits.PEN = TRUE; // Generate stop event.
            break;

            case wait_done: // Wait for slave to detect stop.
            putcUART1('F');
            state = done; // Slave detected stop.
            buffer.ready = TRUE; // Buffer is ready to be read.
            break;

            default: // handle error?
            putcUART1('G');
            break;
        }
    }
    if(INTGetFlag(INT_I2CB)) {
        putcUART1('H');
    }
    INTClearFlag(INT_I2C);
}

void requestStartTransfer(BOOL restart) {
    buffer.index = 0;
    while(I2CCONbits.SEN || I2CCONbits.PEN || I2CCONbits.RSEN || I2CCONbits.RCEN 
        || I2CCONbits.ACKEN || I2CSTATbits.TRSTAT);
    if(restart) {
        I2CCONbits.RSEN = TRUE; // Set repeated start event.
    } else {
        I2CCONbits.SEN = TRUE; // Start condition
    }
    buffer.ready = FALSE;
    state = send_header;
}

void requestWriteByte(UINT8 slave_address, UINT8 reg_address, UINT8 data) {
    getMessageHeader(buffer.header, slave_address, reg_address);
    buffer.wdata[0] = data;
    buffer.mode = WRITE_MODE;
    buffer.len = 1;
    // Check if it's safe to start transfer!
    requestStartTransfer(FALSE);
}

void requestWriteBytes(UINT8 slave_address, UINT8 reg_address, UINT8* data_arr, UINT8 len) {
    getMessageHeader(buffer.header, slave_address, reg_address);
    // Copy data from data_arr to the buffer so it doesn't matter what happens to data_arr
    copyArray(data_arr, buffer.wdata, len);
    buffer.len = len;
    buffer.mode = WRITE_MODE;
    // Check if it's safe to start transfer!
    requestStartTransfer(FALSE);
}

void requestReadByte(UINT8 slave_address, UINT8 reg_address, UINT8* data) {
    getMessageHeader(buffer.header, slave_address, reg_address);
    buffer.rdata = data; // Read directly to variable memory address.
    buffer.len = 1;
    buffer.mode = READ_MODE;
    requestStartTransfer(FALSE);
}

void requestReadBytes(UINT8 slave_address, UINT8 reg_address, UINT8* data_arr, UINT8 len) {
    getMessageHeader(buffer.header, slave_address, reg_address);
    buffer.rdata = data_arr; // Read directly into array memory address.
    buffer.len = len;
    buffer.mode = READ_MODE;
    requestStartTransfer(FALSE);
}