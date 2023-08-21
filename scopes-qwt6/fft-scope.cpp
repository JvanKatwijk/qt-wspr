#
/*
 *    Copyright (C)  2023
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the qt-wspr
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
#include	"spectrum-scope.h"
#include	"waterfall-scope.h"
#include	"fft-scope.h"

//
	fftScope::fftScope (QwtPlot	*plotgrid,
	                    int16_t	displaySize,
	                    int32_t	scale,
	                    int32_t	SampleRate,
	                    int16_t	level,
	                    int16_t	freq) {
int16_t	i;
double	temp;
	this	-> scope		= new spectrumScope (plotgrid, 
	                                                     displaySize);
	viewMode = SPECTRUMVIEW;

	connect (scope, SIGNAL (clickedwithLeft (int)),
	         this,  SIGNAL (clickedwithLeft (int)));
	connect (scope, SIGNAL (clickedwithRight (int)),
	         this,  SIGNAL (clickedwithRight (int)));
	this	-> plotgrid		= plotgrid;
	this	-> displaySize		= displaySize;
	this	-> segmentSize		= SampleRate / freq;
	this	-> scale		= scale;
	this	-> spectrumSize		= 2 * displaySize;
	this	-> sampleRate		= SampleRate;
	this	-> MaxFrequency		= SampleRate / 2;
	this	-> freq			= freq;
	bitDepth			= 8;
	averageCounter			= 0;
	this	-> fillPointer		= 0;
	this	-> vfo			= 0;
	this	-> needle		= 0;
	this	-> displayBuffer	= new double [displaySize];
	this	-> averageBuffer	= new double [displaySize];
	memset (averageBuffer, 0, displaySize * sizeof (double));
	this	-> X_axis		= new double [displaySize];
	this	-> Window		= new float [spectrumSize];
	this	-> sampleCounter	= 0;
	this	-> fftVector_in		= new kiss_fft_cpx [spectrumSize];
        this	-> fftVector_out	= new kiss_fft_cpx [spectrumSize];
        this	-> plan			=
	                kiss_fft_alloc (spectrumSize, true, nullptr, nullptr);

	for (i = 0; i < spectrumSize; i ++) 
	   Window [i] = 0.42 - 0.5 * cos ((2.0 * M_PI * i) / (spectrumSize - 1)) +
	                      0.08 * cos ((4.0 * M_PI * i) / (spectrumSize - 1));


	temp	= (double)MaxFrequency / displaySize;
	for (i = 0; i < displaySize; i ++)
	   X_axis [i] =
	      ((double)vfo - (double)MaxFrequency
	           +  (double)((i) * (double) 2 * temp)) / ((double)scale);
	
	scope	-> set_bitDepth (bitDepth);
}

	fftScope::~fftScope () {
	delete[]	this	-> displayBuffer;
	delete[]	this	-> averageBuffer;
	delete[]	this	-> X_axis;
	delete[]	this	-> Window;
	delete[]	this	-> fftVector_in;
	delete[]	this	-> fftVector_out;
	delete		this	-> scope;
}

static inline
float	abs (kiss_fft_cpx x) {
	return sqrt (x. r * x. r + x. i * x.i);
}

void	fftScope::addFrame (std::complex<float> *v, int32_t n, int amp) {
	for (int i = 0; i < n; i ++) {
	   fftVector_in [i]. r = real (v [i]) * Window [i];
	   fftVector_in [i]. i = imag (v [i]) * Window [i];
	}
	for (int i = n; i < spectrumSize; i ++) {
	   fftVector_in [i]. r =  0;
	   fftVector_in [i]. i =  0;
	}

	kiss_fft (plan, fftVector_in, fftVector_out);
	for (int i = 0; i < displaySize / 2; i ++) {
	   displayBuffer [i] = 0;
	   displayBuffer [displaySize / 2 + i] = 0;
	   for (int j = 0; j < 2; j ++) {
	      displayBuffer [i] +=
	           0.25 * abs (fftVector_out [spectrumSize / 2 + 2 * i + j]);
	      displayBuffer [displaySize / 2 + i] +=
	           0.25 * abs (fftVector_out [2 * i + j]);
	   }
	}

//	X axis labels
	double  temp    = (double)sampleRate / 2 / displaySize;
        for (int i = 0; i < displaySize; i ++)
           X_axis [i] =
                 ((double)vfo - (double)(sampleRate / 2) +
                  (double)((i) * (double) 2 * temp)) / (int32_t)scale;

	scope -> display (X_axis,
	                  displayBuffer,
	                  amp,
	                  0);
}

void	fftScope::switch_viewMode	(void) {
	delete scope;
	if (viewMode == SPECTRUMVIEW) {
	   viewMode = WATERFALLVIEW;
	   scope = new waterfallScope (plotgrid, displaySize, 50);
	}
	else {
	   viewMode = SPECTRUMVIEW;
	   scope = new spectrumScope (plotgrid, displaySize);
	}

	scope	-> set_bitDepth (bitDepth);
	memset (averageBuffer, 0, displaySize * sizeof (double));
	connect (scope, SIGNAL (clickedwithLeft (int)),
	         this,  SIGNAL (clickedwithLeft (int)));
	connect (scope, SIGNAL (clickedwithRight (int)),
	         this,  SIGNAL (clickedwithRight (int)));
}

void	fftScope::set_bitDepth		(int16_t bd) {
	scope	-> set_bitDepth (bd);
	bitDepth	= bd;
}

