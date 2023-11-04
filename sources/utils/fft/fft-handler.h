
#ifndef	__FFT_HANDLER__
#define	__FFT_HANDLER__

#include	<vector>
#include	"kiss_fft.h"

class	fftHandler {
public:
			fftHandler	(int size, bool);
			~fftHandler	();
	void		fft ();
	kiss_fft_cpx	*fftVector_in;
	kiss_fft_cpx	*fftVector_out;
private:
	int		size;
	kiss_fft_cfg	plan;
};
#endif
