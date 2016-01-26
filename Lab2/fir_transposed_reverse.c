#include <usbstk5515.h>
#include <usbstk5515_i2c.h>
#include <AIC_func.h>
#include <stdio.h>
#include "low_pass.h"

#define TCR0 		*((ioport volatile Uint16 *)0x1810)
#define TIMCNT1_0 	*((ioport volatile Uint16 *)0x1814)
#define TIME_START  0x8001
#define TIME_STOP   0x8000

Int16 in; // the most recent input data
Uint16 delta_time;

Int32 sum[LPL]; // sum array which store the internal value
// this value should be stored in a circular buffer structure instead of in[ASIZE]
// "in" is not an array now~
Int32 sum_temp[LPL]; // sum_temp[] is use to initialization

// Implement Transposed Form FIR filter
Int16 FIR_T()
{
	Uint16 k;

	//The actual filter work (Transposed Form)
	for(k = 0; k < LPL - 1; k++)
	{
		sum[k] = (Int32)in * (Int32)LP[k] + sum[k + 1];
	}
	sum[LPL - 1] = (Int32)in * (Int32)LP[LPL - 1];

	sum[0] = sum[0] + 0x00004000;	// So we round rather than truncate.
	return (Int16)(sum[0] >> 15);  		// Conversion from 32 Q30 to 16 Q15.
}

void main(void)
{
	Uint16 i, j;
	Uint16 start_time;
	Uint16 end_time;
	Int16  right, left; //AIC inputs
	Int16  out;

	USBSTK5515_init(); 	//Initializing the Processor
	AIC_init(); 		//Initializing the Audio Codec

	//Priming the PUMP (sum[LPL])
	// for previous code, only one output value be correctly initialized!!!
	// Others will not!!! must use two loops!
	for(i = 0; i < LPL; i++)
	{
		AIC_read2(&right, &left);
		in = right;
		sum_temp[0] = (Int32)in * (Int32)LP[LPL - 1];
		for(j = 1; j <= i; j++) // end with j = LPL -1 (out i loop)
		{
			sum_temp[j] = (Int32)in * (Int32)LP[LPL - j - 1] + sum[j - 1];
			// for j = i, is to update the newest sum... (near final output)
		}
		for(j = 0; j <= i; j++)
		{
			sum[j] = sum_temp[j]; // update others as well...
		}
	}

	TCR0 = TIME_STOP;
	TCR0 = TIME_START; //Resets the time register

	while(1)
	{
		AIC_read2(&right, &left);
		in = right;

		//Measuring the time of the filter chosen
		// result is 340 CPU cycles
		start_time = TIMCNT1_0;
		out = FIR_T();
		end_time = TIMCNT1_0;
		delta_time = (start_time - end_time) << 1;

		//POSTFILTER:
		AIC_write2(right, out);
	}
}