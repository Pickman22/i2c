#include "utils.h"


void copyArray(UINT8* from, UINT8* to, UINT8 len) {
	UINT8 idx;
	for(idx = 0; idx < len; idx++) {
		to[idx] = from[idx];
	}
}