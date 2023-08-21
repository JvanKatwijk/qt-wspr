#
/*
 *    Copyright (C)  2023
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
#include	<unistd.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <signal.h>
#include        <getopt.h>
#include        <math.h>
#include        <string.h>
#include        <sys/time.h>

#include	<QSettings>
#include	<QDebug>
#include	<QDateTime>
#include	<QObject>
#include	<QDir>
#include	<QColor>
#include	<QMessageBox>
#include	<QFileDialog>
#include	"constants.h"
#include	"radio.h"
#include	"fft-scope.h"
#include	"decoder.h"
//
//      devices
#include        "device-handler.h"
#ifdef HAVE_SDRPLAY_V2
#include	"sdrplay-handler-v2.h"
#elif HAVE_SDRPLAY_V3
#include	"sdrplay-handler-v3.h"
#elif HAVE_AIRSPY
#include	"airspy-handler.h"
#elif HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#elif HAVE_HACKRF
#include	"hackrf-handler.h"
#elif HAVE_LIME
#include	"lime-handler.h"
#elif HAVE_PLUTO
#include	"pluto-handler.h"
#endif
//

	RadioInterface::RadioInterface (QSettings	*sI,
	                                QWidget		*parent):
	                                    QMainWindow (parent),
	                                    decimator_0 (17, -1000,
	                                                   1000, 192000, 16),
	                                    decimator_1 (11, -1000,
	                                                   1000, 12000, 4),
	                                    decimator_2 (5,  -150,
	                                                      150, 3000, 8),
	                                    inputBuffer (32 * 32768),
	                                    passBuffer (16 * 32768) {
struct timeval lTime;
	this	-> settings	= sI;
	setupUi (this);
//
//	and the decoders
	displaySize		= 1024;
	frequencyDisplay	->  display (KHz (14096));
	mouseIncrement		= 5;
//	scope and settings
	hfScopeSlider	-> setValue (50);
	lfScopeSlider	-> setValue (50);
	lfScope		= new fftScope (lfSpectrum,
	                                displaySize,
	                                1,        // scale
	                                512,
	                                lfScopeSlider -> value (),
	                                10);
	lfScope		-> set_bitDepth (12);	// default
	hfScope		= new fftScope (hfSpectrum,
	                                displaySize,
	                                1,        // scale
	                                4096,
	                                hfScopeSlider -> value (),
	                                10);
	hfScope		-> set_bitDepth (12);	// default
	bufferIndex_3000		= 0;
	bufferIndex_375			= 0;
	savingSamples. store (false);
	statusLabel	-> setStyleSheet ("QLabel {background-color:green}");
	statusLabel	-> setText ("waiting");
	sampleCount			= 0;

	changeRequest. state	= false;
	changeRequest. freqChange	= false;
	connect (hfScope,
	         SIGNAL (clickedwithLeft (int)),
	         this,
	         SLOT (adjustFrequency (int)));
	connect (hfScope,
	         SIGNAL (clickedwithRight (int)),
	         this, SLOT (switch_hfViewMode (int)));
	connect (lfScope,
	         SIGNAL (clickedwithLeft (int)),
	         this,
	         SLOT (adjustFrequency (int)));
	connect (lfScope,
	         SIGNAL (clickedwithRight (int)),
	         this, SLOT (switch_lfViewMode (int)));
	connect (mouse_Inc, SIGNAL (valueChanged (int)),
	         this, SLOT (set_mouseIncrement (int)));
	connect (freqButton, SIGNAL (activated (const QString &)),
	         this, SLOT (handle_freqButton (const QString &)));
	connect (quickMode_Button, SIGNAL (clicked ()),
	         this, SLOT (handle_quickMode_Button ()));
	connect (subtraction_Button, SIGNAL (clicked ()),
	         this, SLOT (handle_subtraction_Button ()));
	connect (hashtable_Button, SIGNAL (clicked ()),
	         this, SLOT (handle_hashTable_Button ()));
	connect (report_Button, SIGNAL (clicked ()),
	         this, SLOT (handle_report_Button ()));
	connect (identity_Button, SIGNAL (clicked ()),
	         this, SLOT (handle_identity_Button ()));
	connect (npasses_Selector, SIGNAL (valueChanged (int)),
	         this, SLOT (handle_npasses_Selector (int)));

	myDecoder	= new decoder (this, &passBuffer);

	try {
	   theDevice = new sdrplayHandler_v3 (this, &inputBuffer,
	                                           settings, 192000);
	   } catch (int e) {
	      return;
	}

	QString selectedBand	=
	         settings	-> value ("selected band", "LF"). toString ();
	int k	= freqButton -> findText (selectedBand);
	if (k != -1)
	   freqButton -> setCurrentIndex (k);
	displayTimer. setInterval (1000);
	connect (&displayTimer, SIGNAL (timeout ()),
	         this, SLOT (updateTime ())); 
	displayTimer. start (1000);
//
//	Initialize decoder and device settings
	decOptions. usehashtable	= false;
	hashtable_display	-> setText ("hash table off");
	decOptions. npasses		= 2;
	npasses_Selector	-> setValue (2);
	fprintf (stderr, "npasses gezet\n");
	decOptions. subtraction		= true;
	subtraction_display	-> setText ("subtraction on");
	decOptions. quickmode		= false;
	quickMode_display	-> setText ("quick mode off");
	decOptions. report		= false;
	report_display		-> setText ("report off");
	decOptions. dialfreq		= KHz (14096);	// default
//

	connect (theDevice, SIGNAL (dataAvailable (int)),
	        this, SLOT (handle_dataAvailable (int)));
	connect (&secondsTimer, SIGNAL (timeout ()),
	         this, SLOT (twoMinutes ()));

	theDevice	-> setVFOFrequency (arg_to_freq (freqButton -> currentText ()) + 1500) ;
	
	frequencyDisplay ->  display (theDevice -> getVFOFrequency ());
	theDevice	-> restartReader ();
	gettimeofday (&lTime, nullptr);
	uint32_t sec		= lTime.tv_sec % 120;
	uint32_t usec		= sec * 1000000 + lTime.tv_usec;
	uint32_t uwait		= 120000000 - usec - 2000;
	secondsTimer. setSingleShot (true);
	secondsTimer. start (uwait / 1000);
}
//      The end of all
	RadioInterface::~RadioInterface () {
}

//
//	If the user quits before selecting a device ....
void	RadioInterface::handle_quitButton	() {
	if (theDevice != nullptr) {	// should be the case
	   theDevice	-> stopReader ();
	   disconnect (theDevice, SIGNAL (dataAvailable (int)),
	               this, SLOT (handle_dataAvailable (int)));
	   delete  theDevice;
	}
	sleep (1);
	fprintf (stderr, "device is deleted\n");
	secondsTimer. stop ();
	delete		hfScope;
	delete		lfScope;
}
//
//	

void	RadioInterface::set_mouseIncrement (int inc) {
	mouseIncrement = inc;
}

void	RadioInterface::wheelEvent (QWheelEvent *e) {
bool forward;
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 0)
	forward = e -> angleDelta (). y () > 0;
#else
	forward = e -> delta () > 0;
#endif
	adjustFrequency (forward ?
	                        mouseIncrement : -mouseIncrement);
}

	
//

void	RadioInterface::switch_hfViewMode	(int d) {
	(void)d;
	hfScope	-> switch_viewMode ();
}

void	RadioInterface::switch_lfViewMode	(int d) {
	(void)d;
	lfScope	-> switch_viewMode ();
}

void	RadioInterface::updateTime		() {
QDateTime currentTime = QDateTime::currentDateTime ();
static int secondsCounter	= 0;

	if (savingSamples) {
	   secondsCounter ++;
	   statusLabel -> setText ("reading " + QString::number (secondsCounter));
	}
	else
	   secondsCounter = 0;
	timeDisplay     -> setText (currentTime.
	                            toString (QString ("dd.MM.yy:hh:mm:ss")));
}

#include <QCloseEvent>
void RadioInterface::closeEvent (QCloseEvent *event) {

	QMessageBox::StandardButton resultButton =
	                QMessageBox::question (this, "dabRadio",
	                                       tr("Are you sure?\n"),
	                                       QMessageBox::No | QMessageBox::Yes,
	                                       QMessageBox::Yes);
	if (resultButton != QMessageBox::Yes) {
	   event -> ignore();
	} else {
	   handle_quitButton ();
	   event -> accept ();
	}
}

void	RadioInterface::handle_freqButton	(const QString &s) {
int	freq	= arg_to_freq (s);
	changeRequest. state 		= true;
	changeRequest. freqChange	= true;
	changeRequest. freqValue	= freq;
	QMessageBox::warning (this, tr ("Warning"),
	                     tr ("The change will be effective at the next cycle"));
	settings	-> setValue ("selected band", s);
}
//
//	The device handles decimation to a decent 192000 samples/sec
//	and passes slices of 1920 samples on.
//	delay will be between 1 and 2 milliseconds
void	RadioInterface::handle_dataAvailable (int n) {
int	amp	= lfScopeSlider	-> value ();
	n = inputBuffer. GetRingBufferReadAvailable ();
	for (int i = 0; i < n; i ++) {
	   std::complex<float> sample;
	   inputBuffer. getDataFromBuffer (&sample, 1);
	   if (decimator_0. Pass (sample, &sample) &&
	       decimator_1. Pass (sample, &sample)) {
	       buffer_3000 [bufferIndex_3000 ++] = sample;
	       if (bufferIndex_3000 >= 1024) {
	          hfScope -> addFrame (buffer_3000, 1024, amp);
	          bufferIndex_3000 = 0;
	      }
	      if (decimator_2. Pass (sample * 10.0f, &sample) ) {
	         buffer_375 [bufferIndex_375 ++] = sample;
	         if (bufferIndex_375 >= 512) {
	            lfScope -> addFrame (buffer_375, 512, amp);
	            bufferIndex_375 = 0;
	            for (int i = 375; i < 512; i ++)
	               buffer_375 [bufferIndex_375 ++] = buffer_375 [i];
	         }
	         if (savingSamples. load ()) {
	            passBuffer. putDataIntoBuffer (&sample, 1);
	            sampleCount ++;
	            if (sampleCount >= SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE) {
	               struct timeval lTime;
	               savingSamples. store (false);
	               statusLabel -> setStyleSheet ("QLabel {backgroundcolor: red}");
	               statusLabel -> setText ("decoding");
	               sampleCount = 0;
	               gettimeofday (&lTime, nullptr);
	               uint32_t sec		= lTime.tv_sec % 120;
	               fprintf (stderr, "gestopt at %d\n", sec);
	               myDecoder	-> goDecoding (decOptions);
	               statusLabel	-> setStyleSheet ("QLabel {backgroundcolor: green}");
	               statusLabel	-> setText ("waiting");
	               sec		= lTime.tv_sec % 120;
	               uint32_t usec		= sec * 1000000 + lTime.tv_usec;
	               uint32_t uwait		= 120000000 - usec - 2000;
	               secondsTimer. start (uwait / 1000);
	            }
	         }
	         else 
	         if (changeRequest. state) {
	            if (changeRequest. freqChange) {
	               theDevice ->
	                   setVFOFrequency (changeRequest. freqValue + 1500);
	               frequencyDisplay	->  display (changeRequest. freqValue);
	               changeRequest. freqChange = false;
	            }
	         }
	      }
	   }
	}
}

void	RadioInterface::twoMinutes	() {
	secondsTimer. stop ();
	struct timeval lTime;
	gettimeofday (&lTime, nullptr);
	fprintf (stderr, "starting the decoder at %d\n",
	                                (int)(lTime.tv_sec % 120));
	secondsTimer. start (120 * 1000);
	savingSamples. store (true);
	statusLabel	-> setStyleSheet ("QLabel {background-color: blue}");
	statusLabel	-> setText ("reading");
}

struct {
	QString key;
	int	freq;
} freqTable [] = {
	{"LF", 136000},	   {"jan", 144100000}, {"MF", 474200},
	{"160m", 1836600}, {"80m", 3568600},  {"60m", 5287200},
	{"40m", 7038600},  {"30m", 10138700}, {"20m", 14095600},
	{"17m", 18104600}, {"15m", 21094600}, {"12m", 24924600},
	{"10m", 28124600}, {"6m", 50293000},  {"4m", 70091000},
	{"2m", 144489000}, {"70cm",  432300000},
	{"", -1}
};

int32_t	RadioInterface::arg_to_freq (const QString &s) {
	for (int i = 0; freqTable [i]. key != ""; i ++)
	   if (s == freqTable [i]. key)
	      return freqTable [i]. freq;
	return 14095600;
}

void	RadioInterface::adjustFrequency (int n) {
	theDevice -> setVFOFrequency (theDevice -> getVFOFrequency () - n / 2);
	frequencyDisplay	->  display (theDevice -> getVFOFrequency ());
}

void	RadioInterface::handle_quickMode_Button () {
	decOptions. quickmode		= !decOptions. quickmode;
	quickMode_display	-> setText (decOptions. quickmode ?
	                                      "quickMode On" : "quickMode off");
}

void	RadioInterface::handle_subtraction_Button       () {
	decOptions. subtraction		= !decOptions. subtraction;
	subtraction_display	-> setText (decOptions. subtraction ?
	                                   "subtraction On" : "subtraction off");
}

void	RadioInterface::handle_hashTable_Button () {
	if (savingSamples. load ()) {
	   decOptions. usehashtable	= !decOptions. usehashtable;
	   hashtable_display	-> setText (decOptions. usehashtable ?
	                                   "hashtable on" : "hashtable off");
	}
	else
	   QMessageBox::warning (this, tr ("Warning"),
	                        tr ("Only effecive during reading samples"));
}

void	RadioInterface::handle_report_Button    () {
	decOptions. report	= !decOptions. report;
	report_display		-> setText (decOptions. report ?
	                                    "report on": "report off");
}

void	RadioInterface::handle_identity_Button  () {}
 
void	RadioInterface::handle_npasses_Selector (int n) {
	decOptions. npasses = n;
}

void	RadioInterface::showText (const QStringList &resList) {
	model. clear ();
	for (auto res : resList)
	   model. appendRow (new QStandardItem (res));
	textDisplay -> setModel (&model);
}

void    RadioInterface::printLine (const QString &s) {
	if (theResults. size () >= 10)
	   theResults. pop_front ();
	theResults += s; 
 
//	if (filePointer. load () != nullptr)
//	   fprintf (filePointer, "%s\n", s. toUtf8 (). data ());
//	fprintf (stderr, "%s\n", s. toLatin1 (). data ());
	showText (theResults);
}

