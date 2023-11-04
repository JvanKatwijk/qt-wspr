#
/*
 *    Copyright (C) 2023
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

#include	"fir-filters.h"
#ifndef	__MINGW32__
#include	"alloca.h"
#endif

		basicFIR::basicFIR (int16_t size):
	                                   filterKernel (size),
	                                   Buffer       (size) {
	int16_t		i;
	filterSize	= size;
	ip		= 0;

	for (i = 0; i < filterSize; i ++) {
	   filterKernel [i]	= std::complex<float> (0, 0);
	   Buffer [i]		= std::complex<float> (0, 0);
	}
}

		basicFIR::~basicFIR () {
}
//
std::complex<float>	basicFIR::Pass (std::complex<float> z) {
int16_t	i;
std::complex<float>	tmp	= 0;

	Buffer [ip]	= z;
	for (i = 0; i < filterSize; i ++) {
	   int16_t index = ip - i;
	   if (index < 0)
	      index += filterSize;
	   tmp		+= Buffer [index] * filterKernel [i];
	}

	ip = (ip + 1) % filterSize;
	return tmp;
}

float	basicFIR::Pass (float v) {
int16_t		i;
float	tmp	= 0;

	Buffer [ip] = std::complex<float> (v, 0);
	for (i = 0; i < filterSize; i ++) {
	   int16_t index = ip - i;
	   if (index < 0)
	      index += filterSize;
	   tmp += real (Buffer [index]) * real (filterKernel [i]);
	}

	ip = (ip + 1) % filterSize;
	return tmp;
}

	lowpassFIR::lowpassFIR (int16_t firSize,
	                        int32_t Fc, int32_t fs):
	                                     basicFIR (firSize) {
float	tmp [firSize];
float	f	= (float)Fc /fs;
float	sum	= 0.0;
	sampleRate	= fs;

	for (int i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * f;
	   else 
	      tmp [i] = sin (2 * M_PI * f * (i - filterSize/2))/ (i - filterSize/2);
//
//	Blackman window
	   tmp [i]  *= (0.42 -
		    0.5 * cos (2 * M_PI * (float)i / filterSize) +
		    0.08 * cos (4 * M_PI * (float)i / filterSize));

	   sum += tmp [i];
	}

	for (int i = 0; i < filterSize; i ++)
	   filterKernel [i] = std::complex<float> (tmp [i] / sum, 0);
}

	lowpassFIR::~lowpassFIR () {
}

