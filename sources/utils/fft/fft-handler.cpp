

#include	"fft-handler.h"


	fftHandler::fftHandler	(int size, bool inv) {
	this	-> size		= size;
	fftVector_in            = new kiss_fft_cpx [size];
        fftVector_out           = new kiss_fft_cpx [size];
        plan			= kiss_fft_alloc (size, inv, nullptr, nullptr);
}

	fftHandler::~fftHandler	() {
	delete fftVector_in;
	delete fftVector_out;
}

void	fftHandler::fft		() {
	kiss_fft (plan, fftVector_in, fftVector_out);
}

