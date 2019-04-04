#ifndef   _VGA_HEADER_
#define   _VGA_HEADER_

#include "system.h"

//  VGA Parameter
#define VGA_WIDTH     640
#define VGA_HEIGHT    480

#define PIXEL_ON 	  1
#define PIXEL_OFF	  0

//set pixel (x, y) on
#define Vga_Set_Pixel(x, y)  do{\
	IOWR(PIO_PIXEL_POS_BASE, 0, VGA_WIDTH*y+x);\
	IOWR(PIO_POS_STATE_BASE, 0, PIXEL_ON);\
} while(0)

//set pixel (x, y) off
#define Vga_Clr_Pixel(x, y)  do{\
	IOWR(PIO_PIXEL_POS_BASE, 0, VGA_WIDTH*y+x);\
	IOWR(PIO_POS_STATE_BASE, 0, PIXEL_OFF);\
} while(0)

void Set_Pixel_On_Color(unsigned int R,unsigned int G,unsigned int B)
{
	IOWR(PIO_R_ON_BASE, 0, R);
	IOWR(PIO_G_ON_BASE, 0, G);
	IOWR(PIO_B_ON_BASE, 0, B);
}

void Set_Pixel_Off_Color(unsigned int R,unsigned int G,unsigned int B)
{
	IOWR(PIO_R_OFF_BASE, 0, R);
	IOWR(PIO_G_OFF_BASE, 0, G);
	IOWR(PIO_B_OFF_BASE, 0, B);
}

void init_VGA() {
	Set_Pixel_Off_Color(0,0,00);	        //color black
	Set_Pixel_On_Color(1000,1000,1000);		//color white
	int i, j;
	for(i=0; i<VGA_WIDTH; i++)              //set all pixels not display
		for(j=0; j<VGA_HEIGHT; j++)
			Vga_Clr_Pixel(i, j);
};

#endif //

