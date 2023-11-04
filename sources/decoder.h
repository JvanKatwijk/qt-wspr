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
#ifndef	__DECODER_H
#define	__DECODER_H

#include	<QThread>
#include	"constants.h"
#include	"ringbuffer.h"
#include	<complex>
#include        <math.h>
#include        <string.h>
#include	<mutex>
#include        <sys/time.h>
#include	<QString>
#include	"wsprd.h"
#include	"fft-handler.h"
class	RadioInterface;

class	decoder:public QThread {
Q_OBJECT
public:
		decoder		(RadioInterface *,
	                         RingBuffer<std::complex<float>> *);
		~decoder	();
	void	goDecoding	(struct decoder_options decOptions);
private:
	fftHandler	fft;
	void	run		();
	void	postSpot	(struct decoder_results &);
	void	printSpots	(int);
	RingBuffer<std::complex<float>>	*passBuffer;
	std::complex<float> buffer 	[SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE];
	float	samples_i		[SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE];
	float	samples_q		[SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE];
	struct decoder_options  decOptions;
	struct decoder_results  decResults [50];
     	struct tm       *gtm;
	RadioInterface	*myRadio;
	std::mutex	fileLocker;
signals:
	void	printLine	(const QString &);
	void	transmitMessages	();
	void	sendMessage	(const QString &,	// the call
	                         const QString &,	// the loc
	                         int,			// the freq
	                         int,			// the snr
	                         int			// the time
	                        );
	void	sendString	(const QString &);
};

#endif

