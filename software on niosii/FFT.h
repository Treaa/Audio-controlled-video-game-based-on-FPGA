#ifndef _FFT_HEADER_
#define _FFT_HEADER_

#include "system.h"

// Registers Read/Write define
#define FFT_ADDR(base, value)      IOWR(base, 0, value)
#define FFT_START(base, value) 	   IOWR(base, 1, value)
#define FFT_DONE(base)             IORD(base, 0)
#define FFT_POW(base)              IORD(base, 1)
#define FFT_EXP(base)              IORD(base, 2)

void init_FFT() {
	FFT_START(AUDIO_PROCESS_0_BASE, 0);
	//make sure start is zero for a while
	for(int i = 0; i < 1000000; i++);
}

#endif
