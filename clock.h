/* 
 * File:   clock.h
 * Author: tesla
 *
 * Created on September 27, 2014, 5:18 PM
 */

#ifndef CLOCK_H
#define	CLOCK_H

#define	GetSystemClock()                (80000000ul)
#define	GetPeripheralClock()		(GetSystemClock()/(1 << OSCCONbits.PBDIV))
#define	GetInstructionClock()		(GetSystemClock())

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* CLOCK_H */

