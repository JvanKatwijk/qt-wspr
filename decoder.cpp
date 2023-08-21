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
#include	"decoder.h"
#include	"radio.h"

	decoder::decoder	(RadioInterface *myRadio,
	                         RingBuffer<std::complex<float>> *passBuffer) {
	this	-> myRadio	= myRadio;
	this	-> passBuffer	= passBuffer;
	connect (this, SIGNAL (printLine (const QString &)),
	         myRadio, SLOT (printLine (const QString &)));
}

	decoder::~decoder	() {}
//
//	merely collecitng data for the thread to run
void	decoder::goDecoding	(struct decoder_options options) {
	this -> decOptions	= options;
	passBuffer	-> getDataFromBuffer (buffer,
	                          SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE);

	start ();	// single shot !!!, no looping here
}
//
//	The actual decoding takes place in the function wspr_decode
//	that is executed in a separate thread. As far as I could see, 
//	decoding takes less than 4 seconds, so there is no real
//	need for a separate thread, but from an architectural point
//	of view it seems a more elegant solution
void	decoder::run	() {
int	n_results;

	for (int i = 0; i < SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE; i ++) {
	   samples_i [i]     = real (buffer [i]);
	   samples_q [i]     = imag (buffer [i]);
	}
//
//	optional
//	Normalize the sample @-3dB
	float maxSig = 1e-24f;
	for (int i = 0; i < SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE; i++) {
	   float absI = fabs (samples_i [i]);
	   float absQ = fabs (samples_q [i]);

	   if (absI > maxSig)
	      maxSig = absI;
	   if (absQ > maxSig)
	      maxSig = absQ;
	}

	maxSig = 0.5 / maxSig;
	for (int i = 0; i < SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE; i++) {
	   samples_i [i] *= maxSig;
	   samples_q [i] *= maxSig;
	}
	time_t unixtime;
	time (&unixtime);
	unixtime = unixtime - 120 + 1;
	gtm = gmtime (&unixtime);

//	Search & decode the signal */

	wspr_decode (samples_i,
                     samples_q,
                     SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE,
                     decOptions,
                     decResults,
                     &n_results);
        if (decOptions. report)
           postSpots (n_results);
        printSpots (n_results);
}

void	decoder::postSpots	(int amount) {
}

void	decoder::printSpots	(int amount) {
char	tmp [155];
QString	res;
	if (amount == 0) {
	   sprintf (tmp, "No spot %04d-%02d-%02d %02d:%02dz\n",
	       gtm	-> tm_year + 1900,
	       gtm	-> tm_mon + 1,
	       gtm	-> tm_mday,
	       gtm	-> tm_hour,
	       gtm	-> tm_min);
	   res = QString (tmp);
	   printLine (res);
	   return;
	}
	for (int i = 0; i < amount; i++) {
	   sprintf (tmp, "Spot :  %04d-%02d-%02d %02d:%02d:%02d %6.2f %6.2f %10.6f %2d %7s %6s %2s\n",
	       gtm	-> tm_year + 1900,
	       gtm	-> tm_mon + 1,
	       gtm	-> tm_mday,
	       gtm	-> tm_hour,
	       gtm	-> tm_min,
	       gtm	-> tm_sec,
	       decResults [i]. snr,
	       decResults [i]. dt,
	       decResults [i]. freq,
	       (int)decResults [i]. drift,
	       decResults [i]. call,
	       decResults [i]. loc,
	       decResults [i]. pwr);
	   res	= QString (tmp);
	   printLine (res);
	}
}

