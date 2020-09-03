/*----------------------------------------------------------------------*
 *	LED CUBE 4x4x4														*
 *	with ATtiny85, MAX7219 and SD Card									*
 *								 										*
 *	by Mikolaj Wasacz													*
 *																		*
 *	Microcontroller: AVR ATtiny85										*
 *	Fuse bits: Low:0xC2 High:0xDD Ext:0xFF								*
 *	Clock: 8 MHz Internal Oscillator									*
 *----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*
 *	INCLUDES															*
 *----------------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "PetitFatFs/diskio.h"
#include "PetitFatFs/pff.h"

/*----------------------------------------------------------------------*
 *	FONT DEFINITION														*
 *----------------------------------------------------------------------*/

#define E	10
#define A	11
#define L	12
#define CHAR_NUM	13

const uint8_t font[CHAR_NUM * 2] PROGMEM = {
/*	0     1     2     3     4     5     6     7     8     9     E     A     l	*/
	0x9F, 0xF5, 0xB9, 0xD9, 0x2E, 0xDD, 0x5F, 0xB8, 0xDF, 0xAE, 0xDF, 0xAF, 0x1F,
	0xF0, 0x10, 0x50, 0xF0, 0x70, 0xA0, 0x70, 0xC0, 0xF0, 0xF0, 0x90, 0xF0, 0x10,
/*	Intensity and time	*/
	//0xF0, 0xFA, 0xF0, 0xFA
};

#define BUF_8	0xF0
#define BUF_9	0x7D
//#define OFF_8	0xF0
//#define OFF_9	0x00
//#define OFF_10	0x7D
//#define ON_8	0xF0
//#define ON_9	0x00
//#define ON_10	0x7D

/*----------------------------------------------------------------------*
 *	VARIABLES															*
 *----------------------------------------------------------------------*/

#define FILE_ERR	7
#define TIMING_ERR	8

#define PATH_OFFSET 0
char fileName[] = "01.TXT";
//#define PATH_OFFSET 8
//char fileName[] = "LEDCUBE/01.TXT";

// Buffer for data from SD Card
char buffer[11];

// Milliseconds counter
volatile uint16_t msCnt;

// Button click
volatile uint8_t btnClick;

/*----------------------------------------------------------------------*
 *																		*
 *	FUNCTIONS															*
 *																		*
 *----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*
 *	Turn Watchdog off after reset				 						*
 *----------------------------------------------------------------------*/
void __attribute__((naked)) __attribute__((section(".init3"))) wdtOff()
{
	MCUSR = 0;
	WDTCR = (1<<WDCE) | (1<<WDE);
	WDTCR = 0;
}

/*----------------------------------------------------------------------*
 *	Send command to MAX7219						 						*
 *----------------------------------------------------------------------*/
static void maxSend(uint8_t adr, uint8_t data)
{
	xmit_spi(adr);
	xmit_spi(data);
	SELECT();
	DESELECT();
}

/*----------------------------------------------------------------------*
 *	Prepare an empty frame						 						*
 *----------------------------------------------------------------------*/
static void emptyFrame()
{
	buffer[8] = BUF_8;//font[28];
	buffer[9] = BUF_9;//font[29];
	//buffer[10]= OFF_10;
	
	for (uint8_t i = 0; i < 8; i++)
	{
		buffer[i] = 0;
	}
	
	GTCCR = (1<<PSR0);
	TCNT0 = 0;
	msCnt = 0;
}

/*----------------------------------------------------------------------*
 *																		*
 *	MAIN																*
 *																		*
 *----------------------------------------------------------------------*/

int main (void)
{
	// Configure ports
	DDRB   = (1<<CS) | (1<<MOSI) | (1<<SCK);
	PORTB  = (1<<CS) | (1<<MISO) | (1<<BUTTON);
	
	// Configure USI
	USICR  = (1<<USIWM0) | (1<<USICS1) | (1<<USICLK);
	//USICR = (1<<USIWM0) | (1<<USITC);
	
	// Disable analog comparator
	ACSR   = (1<<ACD);

	// Enable button interrupt
	PCMSK  = (1<<BUTTON);
	GIFR   = (1<<PCIF);
	GIMSK  = (1<<PCIE);
	
	//TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10);

	// Configure Timer0 to generate an interrupt every millisecond
	OCR0A  = 124;
	TCCR0A = (1<<WGM01);
	TCCR0B = (1<<CS01) | (1<<CS00);
	TIMSK  = (1<<OCIE0A) | (1<<TOIE1);
	
	// Give SD Card time to initialize
	_delay_ms(1);

	// Enable interrupts
	sei();
	
	// Initialize MAX7219
	maxSend(0xF, 0x00);
	maxSend(0xB, 0x07);
	maxSend(0xA, 0x0F);
	//maxSend(0x9, 0x00);

	for (uint8_t i = 9; i; i--)
	{
		maxSend(i, 0);
	}
	
	maxSend(0xC, 0x01);
	
	// VARIABLES
	// File system object
	FATFS fs;

	// Animation number (0 - play all)
	uint8_t fileNum = 0;

	// Current animation number when playing all
	uint8_t currFileNum = 1;

	// Counter for displaying numbers
	uint8_t cnt;

	// Repetition counter
	uint8_t r_cnt = 0;

	// Store address for repeating
	DWORD	r_fptr = 0;
	CLUST	r_curr_clust;
	DWORD	r_dsect;

	// Error
	uint8_t err;
	
	// Initialize SD Card
	// Mount
	err = pf_mount(&fs);

	if (!err)
	{
		// Open file with the first animation
		err = pf_open(fileName);

		if (!err)
		{
			uint16_t bytesRead;

			// Read the first frame
			err = pf_read(buffer, sizeof(buffer), &bytesRead);

			if (bytesRead != sizeof(buffer) && !err)
			{
				err = FILE_ERR;
			}
		}
	}

	if (err)
	{
		cnt = 10;
		emptyFrame();
	}
	else
	{
		cnt = 0;
	}

	// MAIN LOOP
	while (1)
	{
		// SEND DATA TO MAX7219
		// Update milliseconds counter
		msCnt = (buffer[8] & 0x0F) << 8 | buffer[9];

		// Send data
		maxSend(0xA, buffer[8] >> 4);
				
		for (uint8_t i = 0; i < 8; i++)
		{
			maxSend(8 - i, buffer[i]);
		}

		// CHECK IF A NUMBER SHOULD BE DISPLAYED
		if (cnt)
		{
			uint8_t layer = (cnt - 1) % 5;
			uint8_t num;

			// Get the character
			if (err)
			{
				num = cnt > 5 ? E : err;
			}
			else if (fileNum)
			{
				num = cnt > 5 ? fileName[PATH_OFFSET] - '0' : fileName[PATH_OFFSET + 1] - '0';
				//num = fileName[1 - (cnt - 1) / 5] - '0';
			}
			else
			{
				num = cnt > 10 ? A : L;
			}

			// Display number on current layer
			if (layer != 0)
			{
				buffer[8 - layer] = pgm_read_byte(&font[num]);
				buffer[4 - layer] = pgm_read_byte(&font[num + CHAR_NUM]);
			}
			/*else
			{
				buffer[8] = OFF_8;//font[28];
				buffer[9] = OFF_9;//font[29];
				buffer[10]= OFF_10;
			}*/

			// Clear previous layer
			if (layer != 4)
			{
				buffer[7 - layer] = 0;
				buffer[3 - layer] = 0;
			}
			/*else
			{
				buffer[8] = ON_8;//font[26];
				buffer[9] = ON_9;//font[27];
				buffer[10]= ON_10;
			}*/

			cnt--;
		}

		// RESET IF AN ERROR OCCURED
		else if (err)
		{
			while (msCnt);

			// Wait for button click
			btnClick = 0;
			while (!btnClick);

			// Enable Watchdog
			cli();
			WDTCR = (1<<WDE);
			while (1);
		}

		// REPEAT ANIMATION IF NECESSARY
		else
		{
			uint8_t repeat = buffer[10];

			if (repeat)
			{
				// Begin command
				if (repeat == 0xFF)
				{
					if (r_fptr)
					{
						err = FILE_ERR;
					}
					else
					{
						// Store current address
						r_fptr = fs.fptr;
						r_curr_clust = fs.curr_clust;
						r_dsect = fs.dsect;
					}
				}

				// Repeat one animation infinetely
				else if (fileNum)
				{
					// Restore the beginning address
					fs.fptr = r_fptr;
					fs.curr_clust = r_curr_clust;
					fs.dsect = r_dsect;
				}

				// Repeat current animation several times
				else
				{
					if (!r_cnt)
					{
						r_cnt = repeat;
					}

					if (--r_cnt)
					{
						// Restore the beginning address
						fs.fptr = r_fptr;
						fs.curr_clust = r_curr_clust;
						fs.dsect = r_dsect;
					}
				}
			}

			if (!err)
			{
				// End of file
				if (fs.fptr == fs.fsize)
				{
					// Repeat one animation
					if (fileNum)
					{
						fs.fptr = 0;
					}

					// Switch to next animation
					else
					{
						currFileNum++;

						if (currFileNum > 99)
						{
							currFileNum = 1;
						}

						fileName[PATH_OFFSET] = currFileNum / 10 + '0';
						fileName[PATH_OFFSET + 1] = currFileNum % 10 + '0';

						// Open file
						err = pf_open(fileName);

						if (err == FR_NO_FILE)
						{
							// Search from the beginning
							currFileNum = 1;
							fileName[PATH_OFFSET] = '0';
							fileName[PATH_OFFSET + 1] = '1';

							// Open file
							err = pf_open(fileName);
						}

						r_fptr = 0;
					}
				}

				if (!err)
				{
					uint16_t bytesRead;

					// Read the next frame
					err = pf_read(buffer, sizeof(buffer), &bytesRead);

					if (bytesRead != sizeof(buffer) && !err)
					{
						err = FILE_ERR;
					}
				}

				/*if (!err)
				{
					if (bytesRead == 1)
					{
						if (fileNum)
						{
							fs.fptr = 0;
						}
						else
						{
							if (!rCnt)
							{
								rCnt = buffer[0];
							}

							if (--rCnt)
							{
								fs.fptr = 0;
							}
							else
							{
								currFileNum++;
								if (currFileNum > 99)
								{
									currFileNum = 1;
								}
								fileName[PATH_OFFSET] = currFileNum / 10 + '0';
								fileName[PATH_OFFSET + 1] = currFileNum % 10 + '0';

								err = pf_open(fileName);
								if (err == FR_NO_FILE)
								{
									currFileNum = 1;
									fileName[PATH_OFFSET] = '0';
									fileName[PATH_OFFSET + 1] = '1';

									err = pf_open(fileName);
								}
							}
						}

						if (!err)
						{
							err = pf_read(&buffer, sizeof(buffer), &bytesRead);
						}
					}
				
					if (bytesRead != sizeof(buffer) && !err)
					{
						err = FILE_SIZE_ERR;
					}
				}*/
			}

			if (err)
			{
				cnt = 10;
				emptyFrame();
			}
		}

		// CHECK IF TIMING ERROR OCCURED
		if (!msCnt && !err)
		{
			err = TIMING_ERR;
			cnt = 10;
			emptyFrame();
		}

		// WAIT BEFORE DISPLAYING THE NEXT FRAME
		do
		{
			// REACT TO BUTTON CLICK
			if (btnClick && !err)
			{
				btnClick = 0;

				// Switch to next animation
				fileNum++;

				if (fileNum > 99)
				{
					fileNum = 0;
					currFileNum = 1;
					fileName[PATH_OFFSET] = '0';
					fileName[PATH_OFFSET + 1] = '1';

					// Open file
					err = pf_open(fileName);

					r_cnt = 0;

					cnt = 15;
				}
				else
				{
					fileName[PATH_OFFSET] = fileNum / 10 + '0';
					fileName[PATH_OFFSET + 1] = fileNum % 10 + '0';

					// Open file
					err = pf_open(fileName);

					if (err == FR_NO_FILE)
					{
						// Search from the beginning
						fileNum = 0;
						currFileNum = 1;
						fileName[PATH_OFFSET] = '0';
						fileName[PATH_OFFSET + 1] = '1';

						// Open file
						err = pf_open(fileName);

						r_cnt = 0;

						cnt = 15;
					}
					else
					{
						cnt = fileName[0] == '0' ? 5 : 10;
					}
				}

				r_fptr = 0;
				buffer[10] = 0;
				
				if (err)
				{
					cnt = 10;
				}

				emptyFrame();
			}
		} while (msCnt);
	}
}

/*----------------------------------------------------------------------*
 *																		*
 *	INTERRUPT SERVICE ROUTINES											*
 *																		*
 *----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*
 *	Decrement milliseconds counter				 						*
 *----------------------------------------------------------------------*/
ISR(TIMER0_COMPA_vect)
{
	if (msCnt)
	{
		msCnt--;
	}
}

/*----------------------------------------------------------------------*
 *	Start Timer1 to debounce button				 						*
 *----------------------------------------------------------------------*/
ISR(PCINT0_vect)
{
	// Disable button interrupt
	GIMSK = 0;
	GTCCR = (1<<PSR1);

	// Start Timer1
	TCNT1 = 0;
	TCCR1 = (1<<CS13) | (1<<CS10);
}

/*----------------------------------------------------------------------*
 *	React to button press after debounce time	 						*
 *----------------------------------------------------------------------*/
ISR(TIMER1_OVF_vect)
{
	// Stop Timer1
	TCCR1 = 0;

	// Enable button interrupt
	GIFR  = (1<<PCIF);
	GIMSK = (1<<PCIE);

	// Button status (1 - pressed)
	static uint8_t btnStatus;
	
	if (btnStatus)
	{
		if (PINB & (1<<BUTTON))
		{
			btnStatus = 0;
		}
	}
	else
	{
		if (!(PINB & (1<<BUTTON)))
		{
			btnStatus = 1;
			btnClick = 1;
		}
	}
}