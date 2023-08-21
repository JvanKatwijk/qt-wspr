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

#ifndef __FIR_FILTERS__
#define __FIR_FILTERS__

#include	"constants.h"
#include	<complex>
#include	<vector>

class	basicFIR {
public:
		basicFIR	(int16_t);
		~basicFIR	();
std::complex<float>	Pass	(std::complex<float>);
float		Pass		(float);
void		setTaps		(int16_t len,
	                         float *itaps, float *qtaps);
	std::vector<std::complex<float>> filterKernel;
	std::vector<std::complex<float>> Buffer;
	int	filterSize;
	int	ip;
	int	sampleRate;
};

class	lowpassFIR : public basicFIR {
public:
			lowpassFIR (int16_t,	// order
	                            int32_t, 	// cutoff frequency
	                            int32_t	// samplerate
	                           );
			~lowpassFIR (void);
	std::complex<float>	*getKernel	(void);
	void		newKernel	(int32_t);	// cutoff
};
//
#endif

