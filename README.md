

-----------------------------------------------------------------------------
Qt-wspr, a WSPR listener with a Qt based front end
-----------------------------------------------------------------------------

![overview](/qt-wspr-screen.png?raw=true)

In wspr, the transmitter transmits - starting on the even minutes -
a smallband signal, usually pretty weak, for about 113 seconds.
(Of course the next minute will be even again, and a new message can
be sent.)
The signal itself has a bandwidth of app 6 Hz, with frequency shifts of
about 1.45 Hz.

wspr signals can be heard (better: seen) on a number of predefined
frequencies, and - obviously - usually one seens on a single frequency
a bunch of messages.

An extract from the messages can be sebd to the so-called *pskReporter*
site. On that site one can see the locations of the transmitters whose messages
are received.

Decoding is tough, happily there is a ready-made set of functions that
will do the hard work (wsprd). It must be stressed that the
copyrights for those functions belongs to Joe Taylor and Steven Franke,
and those functions are included integrally in the code for this program..


-----------------------------------------------------------------------
The main widget
----------------------------------------------------------------------

![overview](/main-widget.png?raw=true)

The main widget contains two parts, the control part on the left,
the message part on the right.

WSPR frequencies are standardized. So the widget contains a selector
with which the *band* can be specified, the software translates that to
a frequency.
On the picture, it shows that the 30m band is selected and the selected
frequency is 10140200 Hz. The program des not provide a facility for
tuning, minor deviations of the selected frequency can be obtained
by clicking with the mouse on one of the scopes. The stepsize is 5 Hz,
but can be altered by changing the value on the spinbox on the top line
right.

As said, the messages are transmitted on even seconds. So the software
will wait until the time is right before reading in samples (well actually
all samples are read in, the software just ignores the samples arriving
before or after the message)
The amount of time to wait is displayed on the widget on the green
label.
If the reading phase starts, the number of seconds that samples are read in
is displayed, as shown on the top figure.

Messages take app 112 seconds, the software reads a segment of 116 seconds
in, which covers the 112 period of the message, and is sufficiently small
such that a preparation for starting the read on the next message can be
done.

As said, (part of) each message can be send to the pskReporter. In
order to do that, one needs to have set an identity.
Touching the button labeled *set identity* shows a small widget where
a callsign, a locator and an antenne description can be given.
Of course, this information is stored and meintained between
program invocations.
I am not a real radio amateur, just a listener, so the callsign I am
using is the official "listener"'s number.

If no identity is known to the software, messages are NOT sent to the
pskReporter sire.

The button *report* is used to start (and stop) the software that will
handle the communication to the pskReporter. The picture shows - below
the *report* nutton *report on*, indicating that messages will be sent.

Each time a message is sent, the software displays the result, here apparently
the posting of the message was OK.

Parameters for the decosing process of the wspr messages are

 * quickmode, as the name suggests decoding is faster, but at the price of less accuracy;
 * subtraction, the basic idea here is that once a message has been decoded, it can be encoded again, and the signal of the encoded message is subtracted from
the signal that came in, hopefully to make decosding of other messages possible.
 *hashtable, as the name suggests, results can be stored in a hashtable for
later use
 * number of passes, as can be expeted, a higher number of passes over the
data might result in more accurate decoding. The spinbox with number "2"
here can be used to set that number.

On the second line, the widget contains a button here labeled *WRITING",
indicating that a *dump* of the output is in progress.

The button text normally reads "dump", touching it controls the dumping.
On touching in when dumping, i.e. with the button text "WRITING", dumping
stops. On touching the button when not dumping, a file selection menu
will show, with a *suggested* name already filled in for saving the
output.

The format of the output is in csv format, i.e. the resulting file can be
read with OpenOffice or - when running Windows - the Windows variant
of the software, as shown in the picture below

![overview](/results.png?raw=true)


------------------------------------------------------------------------
Installers and an executable
-------------------------------------------------------------------------

The repository contains both an A[[Image and a Windows installer
Of course it is possible to create an executable,
A qt-wspr.pro file is available for use with qmake and make
There is NO CMakeLists.txt file

-------------------------------------------------------------------------
Copyrights
--------------------------------------------------------------------------

The core of this software, the code implementing the decoding process,
is - apart from some localization options - litterally included.
That code, the code in the "wsprd" subdirectory is copyright
Joe taylor and Steven Franke, with minot modification by Guanael Jouchet.

The copyrights of the KISS library belong to Mark Borgerding and
are gratefully acknowledged.
Of course, the copyrights of the Qt-framework are gratefully acknowledged.

All other stuff copyright J van Katwijk, Lazy Chair Computing

