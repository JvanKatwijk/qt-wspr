#
/*
 *    Copyright (C)  2023
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the qt-wspr decoder
 *
 *    qt-wspr is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    qt-wspr is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with qt-wspr; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#pragma once

#include	<stdint.h>
#include	<complex>
#define	CURRENT_VERSION "0.1"

#define	Khz(x)	(1000 * x)
#define	khz(x)	(1000 * x)
#define	kHz(x)	(1000 * x)
#define	KHz(x)	(1000 * x)

#define	MHz(x)	(1000 * KHz (x))
#define	Mhz(x)	(1000 * KHz (x))

typedef	std::complex<float> Complex;
#ifdef __MINGW32__ 
#include        "windows.h"
#else 
#include        "alloca.h"
#include        "dlfcn.h"
typedef void    *HINSTANCE;
#endif
#ifndef M_PI
# define M_PI           3.14159265358979323846  /* pi */ 
#endif

//
//	device rate:
#define INPUT_SAMPLING_RATE	    (12 * 192000)

#define SIGNAL_LENGTH       116
#define SIGNAL_SAMPLE_RATE  375
//
//	samples for complete message
#define SIGNAL_SAMPLES      SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE
#define NBITS               81
#define NSYM                162
#define NSPERSYM            256
#define DF                  375.0 / 256.0
#define DT                  1.0 / 375.0
#define DF05                DF * 0.5
#define DF15                DF * 1.5
#define TWOPIDT             2.0 * M_PI * DT

/* Option & config of decoder (Shared with the wsprd code) */
struct decoder_options {
	int	dialfreq;	// Dial frequency
	int	freq;		// Dial frequency
	char	rcall [13];	// Callsign of the RX station
	char	rloc[7];	// Locator of the RX station
	int	quickmode;	// Decoder option & tweak
	int	usehashtable;  //  ''
	int	npasses;       //  ''
	int	subtraction;   //  ''
	bool	report;
};

struct cand {
	float  freq;
	float  snr;
	int    shift;
	float  drift;
	float  sync;
};

