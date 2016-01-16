/* QianLong (lonsd) Hello World Driver.*/
/* will copy serial data to lonsd data memory if lonsd started. */
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

/*serial buffer*/
int bufferin = 0; 
int bufferout = 0;
char buffer[1025]; 

/*lonsd define*/
unsigned char far       *drv_type = (unsigned char far *) MK_FP(0x40, 0xf0); /*not 0*/
unsigned char far       *drv_num = (unsigned char far *)MK_FP(0x40, 0xf1); /*5 byte string*/
unsigned char far       *htype = (unsigned char far *)MK_FP(0x40, 0xf6); /*1 for SHASE, 2 for SZNSE, 3 for all*/
unsigned short far * lonsd_seg = (unsigned short far *)MK_FP(0x40, 0xf8); /*lonsd data segment*/
unsigned short far * lonsd_off = (unsigned short far *)MK_FP(0x40, 0xfa);/*lonsd data offset*/
unsigned char far       *data_start = (unsigned char far *)MK_FP(0x40, 0xfc); /*recv data buffer in ptr*/
unsigned char far       *data_end = (unsigned char far *)MK_FP(0x40, 0xfd); /*recv data buffer out ptr*/
unsigned char far *hjzb = (unsigned char far *)MK_FP(0x40, 0xfe); /*reserved , 0xff*/
unsigned char far       *tsr = (unsigned char far *)MK_FP(0x40, 0xff); /*0xaa for tst mark*/

void interrupt (*oldport1isr)();

void interrupt PORT1INT()  /* Interrupt Service Routine (ISR) for PORT1 */

{
	int hasdata = 0;
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

	/* check buffer has 88 bytes */
	if (bufferout == bufferin)hasdata = 0;
	else hasdata = bufferout > bufferin ? (bufferin + 1024 - bufferout >= 88) : (bufferin - bufferout >= 88);

	while (hasdata){
		char far * londadd; /*lonsd data ptr*/
		int i = 0;
		char data[88];
		char far *p;
		for (i = 0; i < 88; i++){
			data[i] = buffer[bufferout];
			bufferout++;
			if (bufferout == 1024) { bufferout = 0; }
		}

		if (*lonsd_seg || *lonsd_off){ /*lonsd started*/
			londadd = (char far *) (MK_FP(*lonsd_seg, *lonsd_off));
			for (i = 0; i < 88; i++){ /*copy data to lonsd data memory*/
				p = (char far *)(londadd + *data_start * 88 + i);
				*p = data[i];
			}

		}
		/*outport(PORT1, *lonsd_seg);*/

		(*data_start)++;
		if (*data_start == 255) {
			*data_start = 0;
		}
		/*outportb(PORT1, '1');*/





		/* check buffer has 88 bytes again*/
		if (bufferout == bufferin)hasdata = 0;
		else hasdata = bufferout > bufferin ? (bufferin + 1024 - bufferout >= 88) : (bufferin - bufferout >= 88);

	}
}




void main(void)
{



	*drv_type = 1;
	*drv_num = '0';
	*(drv_num + 1) = '0';
	*(drv_num + 2) = '0';
	*(drv_num + 3) = '0';
	*(drv_num + 4) = '0';
	*htype = 3;
	*data_start = 0;
	*data_end = 0;
	*tsr = 0;
	*lonsd_seg = 0;
	*lonsd_off = 0;
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

	system("\\ql.bat"); /*call lonsd here*/

}