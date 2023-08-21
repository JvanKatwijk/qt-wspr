#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the wspr decoder
 *
 *    the software is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    the software is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with the software; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<cmath>
#include	"decimator.h"

//
//	decimationFilter.
//	Useful for e.g. reduction of the samplerate with dabsticks

	decimator::decimator (int16_t firSize,
	                      int32_t low,
	                      int32_t high,
	                      int32_t fs,
	                      int16_t Dm):
	                          filterKernel (firSize),
	                          buffer       (firSize) {

float	tmp [firSize];
float	lo	= (float)((high - low) / 2) / fs;
float	shift	= (float) ((high + low) / 2) / fs;
float	sum	= 0.0;

	fprintf (stderr, "decimation from %d with factor %d\n",
	                               fs, Dm);
	filterSize		= firSize;
	ip			= 0;
	sampleRate              = fs;
        decimationFactor        = Dm;
        decimationCounter       = 0;

	for (int i = 0; i < firSize; i ++) {
	   if (i == firSize / 2)
	      tmp [i] = 2 * M_PI * lo;
	   else 
	      tmp [i] = sin (2 * M_PI * lo * (i - firSize /2)) / (i - firSize/2);
//
//	windowing
	   tmp [i]  *= (0.42 -
		        0.5 * cos (2 * M_PI * (float)i / (float)firSize) +
		        0.08 * cos (4 * M_PI * (float)i / (float)firSize));

	   sum += tmp [i];
	}
//
//	and modulate the kernel
	for (int i = 0; i < firSize; i ++) {	// shifting
	   float v = (i - firSize / 2) * (2 * M_PI * shift);
	   filterKernel [i] = std::complex<float> (tmp [i] * cos (v) / sum, 
	                                           tmp [i] * sin (v) / sum);
	}

	for (int i = 0; i < filterSize; i ++)
	   buffer [i] = std::complex<float> (0, 0);
	for (int i = 0; i < firSize; i ++)
	   if (std::isnan (abs (filterKernel [i])))
	      fprintf (stderr, "fout %d\n", i);
}

	decimator::~decimator () {
}
//
//	The real cpu killer: this function is called once for every
//	sample that comes from the dongle. So, it really should be
//	optimized.
bool	decimator::Pass (std::complex<float> z,
	                                 std::complex<float> *z_out) {
int16_t		i;
std::complex<float>	tmp	= 0;
int16_t		index;

	buffer [ip] = z;
	decimationCounter ++;
	if (decimationCounter < decimationFactor) {
	   ip =  (ip + 1) % filterSize;
	   return false;
	}

	decimationCounter = 0;
//
//	we are working with a circular buffer, we take two steps
//	we move from ip - 1 .. 0 with i going from 0 .. ip -1
	for (i = 0; i <= ip; i ++) {
	   index =  ip - i;
	   tmp 	+= buffer [index] * filterKernel [i];
	}
//	and then we take the rest
	for (i = ip + 1; i < filterSize; i ++) {
	   index =  filterSize + ip - i;
	   tmp 	+= buffer [index] * filterKernel [i];
	}

	ip = (ip + 1) % filterSize;
	*z_out = tmp;
	return true;
}

