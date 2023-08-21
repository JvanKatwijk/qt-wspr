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

#ifndef	__FFT_SCOPE__
#define	__FFT_SCOPE__

#include	<QObject>
#include	<stdint.h>
#include	<complex>
#include	<stdexcept>
#include	<iostream>
#include	"virtual-scope.h"
#include	"../kiss/kiss_fft.h"
#

#define	SPECTRUMVIEW	0100
#define	WATERFALLVIEW	0101

class	fftScope : public QObject {
Q_OBJECT
public:
			fftScope	(QwtPlot*,	// the canvas
	                         	 int16_t,	// displayWidth
	                         	 int32_t,	// scale
	                         	 int32_t,	// samplerate
	                                 int16_t,
	                         	 int16_t);	// repeat frequency
			~fftScope	();
	void		addFrame	(std::complex<float> *, int, int);
	void		setLevel	(int16_t);
	void		set_bitDepth	(int16_t);
	void		switch_viewMode	();
private:
	virtualScope	*scope;
	QwtPlot		*plotgrid;

	int32_t		sampleRate;
	int16_t		displaySize;
	int32_t		scale;
	int32_t		fillPointer;
	float		*Window;
	std::complex<float>	*spectrumBuffer;
	kiss_fft_cfg	plan;
	kiss_fft_cpx	*fftVector_in;
	kiss_fft_cpx	*fftVector_out;
	int32_t		sampleCounter;
	int32_t		MaxFrequency;
	int32_t		segmentSize;
	int		averageCounter;
	int32_t		freq;
	int32_t		needle;
	int16_t		amplification;
	int64_t		vfo;
	int16_t		bitDepth;
	uint8_t		viewMode;
	int32_t		spectrumSize;
	double		*X_axis;
	double		*displayBuffer;
	double		*averageBuffer;
signals:
        void            clickedwithLeft         (int);
        void            clickedwithRight        (int);

};
	
#endif

