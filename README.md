

-----------------------------------------------------------------------------
A simple wspr listener with Qt
-----------------------------------------------------------------------------


![overview](/qt-wspr.png?raw=true)

In wspr mode, the transmitter transmits - starting on the even minutes -
a smallband signal, usually pretty weak, for about 113 seconds.
(Of course the next minute will be even again, and a new message can
be sent.

The signal itself has a bandwidth of app 6 Hz, with frequency shifts of
about 1.45 Hz.
Decoding is tough, happily there is a ready-made set of functions that
will do the hard work (wsprd). It must be stressed that the copyrights for those
functions belongs to Joe Taylor and Steven Franke, and those functions
are included integrally in the code for this plugin.

The FFT handling is modified, rather than the FFTW3 library, a choice
was made to use the KISS library, primarily since the sources could be
included, preventing potential users to oblige installing yet another 
library.
The copyrights of the KISS library belong to Mark Borgerding and
are gratefully acknowledged.

Note that this program is made as basis for developing a plugin for SDRuno,
while it functions, it is "still under development"



