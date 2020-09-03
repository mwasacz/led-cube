/*-----------------------------------------------------------------------
/  PFF - Low level disk interface modlue include file    (C)ChaN, 2014
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include "integer.h"


/* Status of Disk Functions */
typedef BYTE	DSTATUS;


/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Function succeeded */
	RES_ERROR,		/* 1: Disk error */
	RES_NOTRDY,		/* 2: Not ready */
	RES_PARERR		/* 3: Invalid parameter */
} DRESULT;

/*----------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified        */
/*----------------------------------------------------------------------*/

#include <avr/io.h>			/* Device include file */

#include <util/delay.h>

//#define xmit_spi(dat) 	SPDR=(dat); loop_until_bit_is_set(SPSR,SPIF)

#define SCK		PB2
#define MISO	PB0
#define MOSI	PB1
#define CS		PB3
#define BUTTON	PB4

BYTE usi_transfer (BYTE data);

#define xmit_spi(dat) usi_transfer(dat)	//SPDR=(dat); loop_until_bit_is_set(SPSR,SPIF)
#define rcv_spi()     usi_transfer(0xFF)

/* Port Controls (Platform dependent) */
#define SELECT()	PORTB &= ~(1<<CS)		/* MMC CS = L */
#define	DESELECT()	PORTB |=  (1<<CS)		/* MMC CS = H */
#define	MMC_SEL		!(PORTB &  (1<<CS))	/* MMC CS status (true:selected) */

/*---------------------------------------*/
/* Prototypes for disk control functions */

DSTATUS disk_initialize (void);
DRESULT disk_readp (BYTE* buff, DWORD sector, UINT offser, UINT count);
DRESULT disk_writep (const BYTE* buff, DWORD sc);

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */

#ifdef __cplusplus
}
#endif

#endif	/* _DISKIO_DEFINED */
