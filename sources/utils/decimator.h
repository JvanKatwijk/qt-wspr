#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the wspr software
 *
 *    this software is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    this software is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this software; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef __DECIMATOR__
#define __DECIMATOR__
#include	<vector>
#include	<complex>

class	decimator {
public:
		         decimator	(int16_t, int32_t,
	                                 int32_t, int32_t, int16_t);
			~decimator	();
	bool		Pass	(std::complex<float>, std::complex<float> *);
private:

	std::vector<std::complex<float>> filterKernel;
	std::vector<std::complex<float>> buffer;
	int	filterSize;
	int	ip;
	int	sampleRate;
	int16_t	decimationFactor;
	int16_t	decimationCounter;
};

#endif

