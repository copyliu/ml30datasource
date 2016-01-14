
 /* Name       : Sample Comm's Program - 1024 Byte Buffer - buff1024.c   */
 /* Written By : Craig Peacock <cpeacock@senet.com.au>                   */

 /*       Copyright 1997 CRAIG PEACOCK <cpeacock@senet.com.au>           */

 /*         See http://www.senet.com.au/~cpeacock/serial1.htm            */
 /*                       For More Information                           */

#include <dos.h>
#include <stdio.h>
#include <conio.h>

#define PORT1 0x3F8  /* Port Address Goes Here */
#define INTVECT 0x0C /* Com Port's IRQ here (Must also change PIC setting) */

  /* Defines Serial Ports Base Address */
  /* COM1 0x3F8                        */
  /* COM2 0x2F8			       */
  /* COM3 0x3E8			       */
  /* COM4 0x2E8			       */

int bufferin = 0;
int bufferout = 0;
char ch;
char buffer[1025];


char far       *scr;
char far       *mode;
unsigned char far       *drv_type = (unsigned char far *) 0x4000f0;
unsigned char far       *drv_num = (unsigned char far *) 0x4000f1;
unsigned char far       *htype = (unsigned char far *) 0x4000f6;
unsigned char far       *data_start = (unsigned char far *) 0x4000fc;
unsigned char far       *data_end = (unsigned char far *) 0x4000fd;
unsigned char far       *tsr = (unsigned char far *) 0x4000ff;
unsigned int far * lonsd_seg = (unsigned int far *) 0x4000f8;
unsigned int far * lonsd_pad = (unsigned int far *) 0x4000fa;
unsigned char far *hjzb = (unsigned char far *) 0x4000fe;

void interrupt (*oldport1isr)();

void interrupt PORT1INT()  /* Interrupt Service Routine (ISR) for PORT1 */
{
	int c;
	do {
		c = inportb(PORT1 + 5);
		if (c & 1) {
			buffer[bufferin] = inportb(PORT1);
			bufferin++;
			if (bufferin == 1024) { bufferin = 0; }
		}
	} while (c & 1);
	outportb(0x20, 0x20);
	while (hasdata()){
		recvdata();
	}
}


int hasdata() {
	if (bufferout==bufferin)return 0;
	if (bufferout > bufferin) /*buffer reseted*/
	{
		if (bufferin + 1024 - bufferout >= 88){
			return 1;
		}
	}
	else{
		if (bufferin - bufferout >= 88){
			return 1;
		}
	}
	return 0;

}
int recvdata(){
	int datacount;
	int datasize;
	char far * londadd;
	int i=0;
	char data[88];
	londadd =(char far *) (MK_FP(*lonsd_seg , *lonsd_pad));
	while (i < 88){
		data[i] = buffer[bufferout];
		bufferout++;
		if (bufferout == 1024) { bufferout = 0; }
		i++;
	}

	if (londadd){
		memcpy(londadd+*data_start*88, data,88);
		outportb(PORT1,*data_start);
		outportb(PORT1,*data_end);
		/*geninterrupt(0x61);*/
	}
	else{



	}
	*data_start++;
	if (*data_start == 255) {
		*data_start = 0;
	}
	outportb(PORT1, '1');
}




void main(void)
{
	int c;




	*drv_type = 1;
	/*
	*drv_num="00000";*/
	strcpy(*drv_num, "00000");
	/*strcpy(*htype, '\x01\x00');*/
	*htype = 3;
	*data_start = 0;
	*data_end = 0;
	*tsr = 0;
	if ((*mode & 0x30) == 0x30)
		scr = (char far *) 0xB0000000;
	else
		scr = (char far *) 0xB8000000;
	*hjzb = 0xff;
	*tsr = 0xaa;





	outportb(PORT1 + 1, 0);        /* Turn off interrupts - Port1 */

	oldport1isr = getvect(INTVECT); /* Save old Interrupt Vector of later
					   recovery */

	setvect(INTVECT, PORT1INT);     /* Set Interrupt Vector Entry */
	/* COM1 - 0x0C */
	/* COM2 - 0x0B */
	/* COM3 - 0x0C */
	/* COM4 - 0x0B */

	/*         PORT 1 - Communication Settings         */

	outportb(PORT1 + 3, 0x80);  /* SET DLAB ON */
	outportb(PORT1 + 0, 0x0C);  /* Set Baud rate - Divisor Latch Low Byte */
	/* Default 0x03 =  38,400 BPS */
	/*         0x01 = 115,200 BPS */
	/*         0x02 =  57,600 BPS */
	/*         0x06 =  19,200 BPS */
	/*         0x0C =   9,600 BPS */
	/*         0x18 =   4,800 BPS */
	/*         0x30 =   2,400 BPS */
	outportb(PORT1 + 1, 0x00);  /* Set Baud rate - Divisor Latch High Byte */
	outportb(PORT1 + 3, 0x03);  /* 8 Bits, No Parity, 1 Stop Bit */
	outportb(PORT1 + 2, 0xC7);  /* FIFO Control Register */
	outportb(PORT1 + 4, 0x0B);  /* Turn on DTR, RTS, and OUT2 */

	outportb(0x21, (inportb(0x21) & 0xEF));  /* Set Programmable Interrupt Controller */
	/* COM1 (IRQ4) - 0xEF  */
	/* COM2 (IRQ3) - 0xF7  */
	/* COM3 (IRQ4) - 0xEF  */
	/* COM4 (IRQ3) - 0xF7  */

	outportb(PORT1 + 1, 0x01);  /* Interrupt when data received */
	keep(0,4000);


	/*
	printf("\nSample Comm's Program. Press ESC to quit \n");

	do {

	if (bufferin != bufferout){ch = buffer[bufferout];
	bufferout++;
	if (bufferout == 1024) {bufferout = 0;}
	printf("%c",ch);}

	if (kbhit()){c = getch();
	outportb(PORT1, c);}

	} while (c !=27);

	outportb(PORT1 + 1 , 0);    */   /* Turn off interrupts - Port1 */
	/*outportb(0x21,(inportb(0x21) | 0x10)); */ /* MASK IRQ using PIC */
	/* COM1 (IRQ4) - 0x10  */
	/* COM2 (IRQ3) - 0x08  */
	/* COM3 (IRQ4) - 0x10  */
	/* COM4 (IRQ3) - 0x08  */
	/*setvect(INTVECT, oldport1isr);*/ /* Restore old interrupt vector */

}