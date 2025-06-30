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

#pragma once

#include        <QMainWindow>
#include        <QWheelEvent>
#include        <QLineEdit>
#include	<QTimer>
#include	<QStandardItemModel>
#include	<sndfile.h>
#include	<mutex>
#include        "ui_newradio.h"
#include        "constants.h"
#include        "ringbuffer.h"
#include	"fir-filters.h"
#include	"decimator.h"

//#include	"wsprd.h"
class		deviceHandler;
class           QSettings;
class           fftScope;
class           fft_scope;
class		decoder;
class		pskReporter;

	struct {
	   bool state;
	   bool	freqChange;
	   int	freqValue;
	} changeRequest;

class	RadioInterface:public QMainWindow,
	               private Ui_MainWindow {
Q_OBJECT
public:
		RadioInterface (QSettings	*sI,
	                        QWidget		*parent = nullptr);
		~RadioInterface	();

private:
	decimator	decimator_0;
	decimator	decimator_1;
	decimator	decimator_2;
	RingBuffer<std::complex<float>> inputBuffer;
	RingBuffer<std::complex<float>> passBuffer;

	decoder		*myDecoder;
	pskReporter	*theReporter;
	QTimer		displayTimer;
	QTimer		secondsTimer;
	int32_t		centerFrequency;
	QSettings       *settings;
        int32_t         inputRate;
	int32_t		scopeWidth;
        int16_t         displaySize;
        int16_t         spectrumSize;
        double          *displayBuffer;
        fftScope	*lfScope;
        fftScope	*hfScope;
	int16_t		mouseIncrement;
	std::complex<float> buffer_375 [512];
	std::complex<float> buffer_3000 [3000];
	int		bufferIndex_375;
	int		bufferIndex_3000;

	struct decoder_options	decOptions;
//	struct decoder_results  decResults [50];
	struct tm	*gtm;
	deviceHandler	*theDevice;
	int		freqFor		(const QString &);
	int		dialFreq;
	int		realfreq;
	int		gain;
	int		lnaState;
	bool		autoGain;
	int		ppm;
	std::mutex	printLock;
	QStandardItemModel      model;
        void            showText        (const QStringList &);
        QStringList     theResults; 
	FILE		*dumpfilePointer;
	std::atomic<bool>	savingSamples;
	int		sampleCount;
	int		arg_to_freq		(const QString &s);

	std::complex<float> buffer		[SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE];
	float		samples_i		[SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE];
	float		samples_q		[SIGNAL_LENGTH * SIGNAL_SAMPLE_RATE];

	deviceHandler	*setDevice		(QSettings *s,
                               			RingBuffer<Complex> *hfBuffer);
	deviceHandler	*getDevice		(const QString &,
	                                         QSettings	*,
	                                         RingBuffer<Complex> *);

private slots:
        void		set_band		(const QString &);
	void		honor_freqRequest	();
        void            wheelEvent              (QWheelEvent *);
	void		set_mouseIncrement	(int);
	void		handle_quitButton	();
	void		switch_hfViewMode	(int);
	void		switch_lfViewMode	(int);
	void		updateTime		();
	void            twoMinutes              ();
	void		closeEvent		(QCloseEvent *event);

	void		handle_quickMode_Button	();
	void		handle_subtraction_Button	();
	void		handle_hashTable_Button	();
	void		handle_report_Button	();
	void		handle_identity_Button	();
	void		handle_dumpButton	();
	void		handle_npasses_Selector	(int);
public slots:
	void		handle_dataAvailable	(int);
	void		adjustFrequency		(int);
	void		printLine		(const QString &);
	void		show_status		(const QString &);
	void		transmitMessages	();
	void		sendMessage		(const QString &,
	                                         const QString &,
	                                         int,
	                                         int,
	                                         int);
	void		sendString		(const QString &);
};

