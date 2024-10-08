// This file is part of LED Cube
// Copyright (C) 2020 Mikolaj Wasacz
// SPDX-License-Identifier: MIT

// This file includes code from Petit FatFs by ChaN, covered by copyright notice given in file pff.c

/*-----------------------------------------------------------------------
/  PFF - Low level disk interface modlue include file    (C)ChaN, 2014
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include "pff.h"


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

#define SCK		PB2
#define MISO	PB0
#define MOSI	PB1
#define CS		PB3
#define BUTTON	PB4

BYTE usi_transfer (BYTE data);

#define xmit_spi(dat)	usi_transfer(dat)
#define rcv_spi()		usi_transfer(0xFF)

/* Port Controls (Platform dependent) */
#define CS_LOW()	PORTB &= ~(1<<CS)	/* Set CS low */
#define	CS_HIGH()	PORTB |=  (1<<CS)	/* Set CS high */
#define	IS_CS_LOW	!(PORTB &  (1<<CS))	/* Test if CS is low */

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
