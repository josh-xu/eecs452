#include <usbstk5515.h>
#include <usbstk5515_gpio.h>
#include <usbstk5515_i2c.h>
#include <usbstk5515_led.h>
#include <stdio.h>
#include <AIC_func.h>
#include <LCD_FCN.h>
#include <sar.h>

#define AIC3204_I2C_ADDR 0x18
#define ASIZE (48*500)		//Number  of entries in array we will display.
#define TSIZE 48			//Number of entries in sin table
#define DELAY 10000

    Int16 sinetable[48] = {
        0x0000, 0x10b4, 0x2120, 0x30fb, 0x3fff, 0x4dea, 0x5a81, 0x658b,
        0x6ed8, 0x763f, 0x7ba1, 0x7ee5, 0x7ffd, 0x7ee5, 0x7ba1, 0x76ef,
        0x6ed8, 0x658b, 0x5a81, 0x4dea, 0x3fff, 0x30fb, 0x2120, 0x10b4,
        0x0000, 0xef4c, 0xdee0, 0xcf06, 0xc002, 0xb216, 0xa57f, 0x9a75,
        0x9128, 0x89c1, 0x845f, 0x811b, 0x8002, 0x811b, 0x845f, 0x89c1,
        0x9128, 0x9a76, 0xa57f, 0xb216, 0xc002, 0xcf06, 0xdee0, 0xef4c
    };

Int16 TO_Display [128];
Int16 Sample_To_Display [ASIZE];     // Place to put data to display on LCD.
									 // It's big so we can zoom out as needed.
Uint16 prompt(void)
{
	Uint16 mode;
    do {
    	fflush(stdin);
    	printf("What would you like to display?  0=1KHz sin, 1=left channel, 2=right channel, 9 to terminate program\n");
    	scanf("%d", &mode);
   } while((mode!=0)&&(mode!=1)&&(mode!=2)&&(mode!=9));
   return mode;
}


Uint16 prompt2(void)
{
	Uint16 mode;
    do {
    	fflush(stdin);
    	printf("What frequency would you like to display?  0=500Hz, 1=750Hz, 2=1kHz, 3=2kHz, 4=3.5kHz\n");
    	scanf("%d", &mode);
   } while((mode!=0)&&(mode!=1)&&(mode!=2)&&(mode!=3)&&(mode!=4));
   return mode;
}


int main (void)
{
	Int16 right, left;  // value from AIC audio input
	Uint16 key;			 // value from Gpain1 SAR (button value)
	Int16  zoom=0; 		 // display scalar (larger is more zoomed in).
	Uint16 mode=0, mode2=0;		 // what we are displaying.
						 // 0=1KHz sin wave, 1=left stereo input, 2=right stereo 
	Uint32 i, j,k ;
	//Initialize devices.
	USBSTK5515_I2C_init( );        // Initialize I2C 
	AIC_init();
	LCD_init();
	Init_SAR();

	j=0xF;
	k=0;
    mode = prompt();
POST_PROMPT:    
   	if(mode==0)
	{
   		mode2 = prompt2(); // definition of "mode2" and "prompt2()" is not included
   		switch(mode2)
		{
   			case 0:
   			{
   				for(i=0; i<ASIZE;) //500Hz
   				{
   					Sample_To_Display[i] = sinetable[k%TSIZE];
   					Sample_To_Display[i+1] = sinetable[k%TSIZE];
   					i=i+2;
   					k++;
   				}
   				break;
   			}
   			case 1:
   			{
   				for(i=0; i<ASIZE;) //750Hz
   				{
   					Sample_To_Display[i] = sinetable[k%TSIZE];
   					Sample_To_Display[i+1] = sinetable[(k+1)%TSIZE];
   					Sample_To_Display[i+2] = sinetable[(k+2)%TSIZE];
   					Sample_To_Display[i+3] = sinetable[(k+2)%TSIZE];
   					i=i+4;
   					k=k+3;
   				}
   				break;
   			}
   			case 2:
   			{
   				for(i=0; i<ASIZE; i++) //1kHz
   				{
   					Sample_To_Display[i] = sinetable[1*i%TSIZE];
   				}
   				break;
   			}
   			case 3:
   			{
   				for(i=0; i<ASIZE; i++) //2kHz
   				{
   					Sample_To_Display[i] = sinetable[2*i%TSIZE];
   				}
   				break;
   			}
   			case 4:
   			{
   				for(i=0; i<ASIZE;) //3.5kHz
   				{
					Sample_To_Display[i] = sinetable[7*k%TSIZE];
					Sample_To_Display[i+1] = sinetable[7*k%TSIZE];
					i=i+2;
					k++;
   				}
   				break;
   			}
		}
	}
	else if(mode==1 || mode==2)
	{
		for(i=0;i<ASIZE;i++)
		{
			AIC_read2(&right, &left);
			if(mode==1)
				Sample_To_Display[i]=left;
			else
				Sample_To_Display[i]=right;
		}
	}
	else if(mode == 9) goto TERMINATE;

   	USBSTK5515_ULED_setall(j);
   	LCD_Display(Sample_To_Display, zoom);
   	i=0;
	while(1)
	{
		if(j & 0x8000)
			j = 0xF;
		if(j > 0xF)
			j = 0;

		key = Get_Key_Human();

		AIC_write2(Sample_To_Display[i], Sample_To_Display[i]);
		i++;
		if(i >= ASIZE) i=0;

		if(key == SW1)
		{
			zoom++;
			j--;
			USBSTK5515_ULED_setall(j);
			LCD_Display(Sample_To_Display, zoom);
			printf("Full Time on the X-axis\n");
			printf("2ms / (2 ^ %d)\n", zoom);
			USBSTK5515_waitusec(DELAY); //wait a bit so we don't grab button again quickly.
		}
		else if(key == SW2)
		{
			zoom--;
			j++;
			USBSTK5515_ULED_setall(j);
			LCD_Display(Sample_To_Display, zoom);
			printf("Full Time on the X-axis\n");
			printf("2ms / (2 ^ %d)\n", zoom);
			USBSTK5515_waitusec(DELAY);
		}
		else if(key == SW12)
		{
			i = zoom = 0;
			mode = prompt();
			goto POST_PROMPT;
		}
			
	}
TERMINATE:
    AIC_off();
    return 0;
}