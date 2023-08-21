
#ifndef	__CONSTANTS_H
#define	__CONSTANTS_H

#include	<stdint.h>
#define	CURRENT_VERSION "0.1"

#define	Khz(x)	(1000 * x)
#define	khz(x)	(1000 * x)
#define	kHz(x)	(1000 * x)
#define	KHz(x)	(1000 * x)

#define	MHz(x)	(1000 * KHz (x))
#define	Mhz(x)	(1000 * KHz (x))

#ifdef __MINGW32__ 
#include        "windows.h"
#else 
#include        "alloca.h"
#include        "dlfcn.h"
typedef void    *HINSTANCE;
#endif
#ifndef M_PI
# define M_PI           3.14159265358979323846  /* pi */ 
#endif

//
//	device rate:
#define INPUT_SAMPLING_RATE	    (12 * 192000)

#define SIGNAL_LENGTH       116
#define SIGNAL_SAMPLE_RATE  375
//
//	samples for complete message
#define SIGNAL_SAMPLES      SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE
#define NBITS               81
#define NSYM                162
#define NSPERSYM            256
#define DF                  375.0 / 256.0
#define DT                  1.0 / 375.0
#define DF05                DF * 0.5
#define DF15                DF * 1.5
#define TWOPIDT             2.0 * M_PI * DT

/* Option & config of decoder (Shared with the wsprd code) */
struct decoder_options {
	int	dialfreq;	// Dial frequency
	int	freq;		// Dial frequency
	char	rcall [13];	// Callsign of the RX station
	char	rloc[7];	// Locator of the RX station
	int	quickmode;	// Decoder option & tweak
	int	usehashtable;  //  ''
	int	npasses;       //  ''
	int	subtraction;   //  ''
	bool	report;
};

struct cand {
	float  freq;
	float  snr;
	int    shift;
	float  drift;
	float  sync;
};

#endif

