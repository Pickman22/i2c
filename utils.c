#include "utils.h"


void copyArray(UINT8* from, UINT8* to, UINT8 len) {
	UINT8 idx;
	for(idx = 0; idx < len; idx++) {
		to[idx] = from[idx];
	}
}

void getMessageHeader(UINT8 *header, UINT8 slave_address, UINT8 reg_address) {
    header[0] = ((slave_address << 1) & 0xFE); // 7bit slave address to 8bit address in write mode.
    header[1] = reg_address;
}