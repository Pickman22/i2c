#ifndef UTILS_H
#define UTILS_H

#include <GenericTypeDefs.h>

void copyArray(UINT8 *from, UINT8 *to, UINT8 len);
void getMessageHeader(UINT8 *header, UINT8 slave_address, UINT8 reg_address);

#endif // UTILS_H