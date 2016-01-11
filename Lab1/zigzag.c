/*
 * main.c
 *
 *      Author: GSI
 */

#include <usbstk5515.h>
#include <usbstk5515_i2c.h>
#include <stdio.h>
#include <LCD_FCN.h>

Uint8 top [128];
Uint8 bottom [128];

void main(void)
{
	Uint16 i;

	USBSTK5515_init();//Initializing the Processer //// included in "usbstk5515.h"???
	LCD_init(); //Initializing the LCD and I2C //// included in "LCD_FCN.h"???
	//Making the zig-zag
	
	//// assume Data set "1" display, set "0" do not display???
	
	//// 0~63 column
	for(i = 0; i<16; i++)
	{
		if(i<8)
		{
			top[i] = 0x01 << i;
			bottom[i] = 0;
		}
		else
		{
			top[i] = 0;
			bottom[i] = 0x01 << (i-8);
		}
	}
	for(i = 16; i<32; i++)
	{
		if(i<24)
		{
			top[i] = 0;
			bottom[i] = 0x80 >> (i-16);
		}
		else
		{
			top[i] = 0x80 >> (i-24);
			bottom[i] = 0;
		}
	}
	for(i=32; i<48; i++)
	{
		if(i<40)
		{
			top[i] = 0x01 << (i-32);
			bottom[i] = 0;
		}
		else
		{
			top[i] = 0;
			bottom[i] = 0x01 << (i-40);
		}
	}
	for(i=48; i<64; i++)
	{
		if(i<56)
		{
			top[i] = 0;
			bottom[i] = 0x80 >> (i-48);
		}
		else
		{
			top[i] = 0x80 >> (i-56);
			bottom[i] = 0;
		}
	}

	//// 64~127 column
	for(i = 0; i<16; i++)
	{
		if(i<8)
		{
			top[i + 64] = 0x01 << i;
			bottom[i + 64] = 0;
		}
		else
		{
			top[i + 64] = 0;
			bottom[i + 64] = 0x01 << (i-8);
		}
	}
	for(i = 16; i<32; i++)
	{
		if(i<24)
		{
			top[i + 64] = 0;
			bottom[i + 64] = 0x80 >> (i-16);
		}
		else
		{
			top[i + 64] = 0x80 >> (i-24);
			bottom[i + 64] = 0;
		}
	}
	for(i=32; i<48; i++)
	{
		if(i<40)
		{
			top[i + 64] = 0x01 << (i-32);
			bottom[i + 64] = 0;
		}
		else
		{
			top[i + 64] = 0;
			bottom[i + 64] = 0x01 << (i-40);
		}
	}
	for(i=48; i<64; i++)
	{
		if(i<56)
		{
			top[i + 64] = 0;
			bottom[i + 64] = 0x80 >> (i-48);
		}
		else
		{
			top[i + 64] = 0x80 >> (i-56);
			bottom[i + 64] = 0;
		}
	}

	//Done making the zig-zag
	printf("Here we go\n");

	//// OSD9616_send(,) function, where defined???
	//// first var means??? 0x00/0x40???? secodn var means???
	OSD9616_send(0x00,0x00);
	OSD9616_send(0x00,0x10);
	OSD9616_send(0x00,0xb0);   // Set page for page pointer to page 0
	for(i=0; i<128; i++)
	{
		OSD9616_send(0x40, top[i]); // Writes the top page //// 128 * 64 dots -> XXX -> 8 bits 8 Pages = 64 dots in one column...
		//// 8 bits 2 Pages = 16 dots in one column -> 128 * 16 dots!!! (actual LCD in lab... Orz...)
		//// ******** 128 columns!!! 16 rows!!! ********
	}
	OSD9616_send(0x00,0x00);
	OSD9616_send(0x00,0x10);
	OSD9616_send(0x00,0xb1);   // Set page for page pointer to page 1
	for(i=0; i<128; i++)
	{
		OSD9616_send(0x40, bottom[i]);// Writes the bottom page
	}
}
