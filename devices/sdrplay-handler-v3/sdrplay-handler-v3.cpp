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
 *    the Free Software Foundation version 2 of the License.
 *
 *    qt-wspr is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with qt-wspr if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	<QThread>
#include	<QSettings>
#include	<QTime>
#include	<QDate>
#include	<QLabel>
#include	<QFileDialog>
#include	"sdrplay-handler-v3.h"
#include	"sdrplay-commands.h"

//	The Rsp's
#include	"Rsp-device.h"
#include	"Rsp1A-handler.h"
#include	"RspII-handler.h"
#include	"RspDuo-handler.h"
#include	"RspDx-handler.h"

#define SDRPLAY_RSP1_   1
#define SDRPLAY_RSP1A_  255
#define SDRPLAY_RSP2_   2
#define SDRPLAY_RSPduo_ 3
#define SDRPLAY_RSPdx_  4

	sdrplayHandler_v3::sdrplayHandler_v3  (RadioInterface *mr,
	                                       RingBuffer<std::complex<float>> *_I_Buffer, 
	                                       QSettings *s, 
	                                       int outputRate):
	                                          myFrame (nullptr) {
	(void)mr;
	this	-> _I_Buffer		= _I_Buffer;
	sdrplaySettings			= s;
	inputRate			= Khz (12 * 192);
	setupUi (&myFrame);
	myFrame. show	();
//
//	decimation factor is 12, i.e. 3 * 4
	filter_1        = new decimator (2 * 4 + 1,
                                              - outputRate / 2,
                                              + outputRate / 2,
                                              inputRate, 3);
        filter_2        = new decimator (2 * 8 + 1,
                                             - outputRate / 2,
                                             + outputRate / 2,
                                             inputRate / 3, 4);


	antennaSelector		-> hide	();
	tunerSelector		-> hide	();
	nrBits			= 12;	// default
	denominator		= 2048;	// default

//	See if there are settings from previous incarnations
//	and config stuff

	sdrplaySettings		-> beginGroup ("sdrplaySettings_v3");
	
	GRdBSelector 		-> setValue (
	            sdrplaySettings -> value ("sdrplay-ifgrdb", 20). toInt());
	GRdBValue		= GRdBSelector -> value ();

	lnaGainSetting		-> setValue (
	            sdrplaySettings -> value ("sdrplay-lnastate", 4). toInt());

	lnaState		= lnaGainSetting -> value ();

	ppmControl		-> setValue (
	            sdrplaySettings -> value ("sdrplay-ppm", 0). toInt());

	agcMode		=
	       sdrplaySettings -> value ("sdrplay-agcMode", 0). toInt() != 0;
	sdrplaySettings	-> endGroup	();

	theRsp		= nullptr;
	if (agcMode) {
	   agcControl -> setChecked (true);
	   GRdBSelector         -> hide ();
	   gainsliderLabel      -> hide ();
	}

	biasT           =
               sdrplaySettings -> value ("biasT", 0). toInt () != 0;
        if (biasT)
           biasT_selector -> setChecked (true);
	
//	and be prepared for future changes in the settings
	connect (GRdBSelector, SIGNAL (valueChanged (int)),
	         this, SLOT (set_ifgainReduction (int)));
	connect (lnaGainSetting, SIGNAL (valueChanged (int)),
	         this, SLOT (set_lnagainReduction (int)));
	connect (agcControl, SIGNAL (stateChanged (int)),
	         this, SLOT (set_agcControl (int)));
	connect (ppmControl, SIGNAL (valueChanged (int)),
	         this, SLOT (set_ppmControl (int)));
	connect (antennaSelector, SIGNAL (textActivated (const QString &)),
	         this, SLOT (set_selectAntenna (const QString &)));
	connect (biasT_selector, SIGNAL (stateChanged (int)),
	         this, SLOT (set_biasT (int)));

	vfoFrequency	= MHz (220);
	theGain		= -1;
	debugControl	-> hide ();
	failFlag. store (false);
	successFlag. store (false);
	errorCode	= 0;
	start ();
	while (!failFlag. load () && !successFlag. load () && isRunning ())
	   usleep (1000);
	if (failFlag. load ()) {
	   while (isRunning ())
	      usleep (1000);
	   throw (errorCode);
	}
	
	fprintf (stderr, "setup sdrplay v3 seems successfull\n");
}

	sdrplayHandler_v3::~sdrplayHandler_v3 () {
	threadRuns. store (false);
	while (isRunning ())
	   usleep (1000);
//	thread should be stopped by now
	myFrame. hide ();
	sdrplaySettings	-> beginGroup ("sdrplaySettings_v3");
	sdrplaySettings -> setValue ("sdrplay-ppm",
	                                           ppmControl -> value ());
	sdrplaySettings -> setValue ("sdrplay-ifgrdb",
	                                           GRdBSelector -> value ());
	sdrplaySettings -> setValue ("sdrplay-lnastate",
	                                           lnaGainSetting -> value ());
	sdrplaySettings	-> setValue ("sdrplay-agcMode",
	                                  agcControl -> isChecked() ? 1 : 0);
	sdrplaySettings	-> endGroup ();
	sdrplaySettings	-> sync();
	if (theRsp != nullptr)
	   delete theRsp;
}

int	sdrplayHandler_v3::getRate	() {
	return inputRate;
}

/////////////////////////////////////////////////////////////////////////
//	Implementing the interface
/////////////////////////////////////////////////////////////////////////

int32_t	sdrplayHandler_v3::defaultFrequency	() {
	return Khz (137100);
}

bool	sdrplayHandler_v3::legalFrequency	(int freq) {
	return (MHz (100) <= freq) && (freq <= MHz (250));
}

int32_t	sdrplayHandler_v3::getVFOFrequency	() {
	return vfoFrequency;
}

void	sdrplayHandler_v3::setVFOFrequency	(int freq) {
restartRequest r (freq);
	vfoFrequency	= freq;
        messageHandler (&r);
	lnaGainSetting -> setMaximum (theRsp -> lnaStates (freq));
}
	
bool	sdrplayHandler_v3::restartReader	() {
restartRequest r (vfoFrequency);

        if (receiverRuns. load ())
           return true;
	return messageHandler (&r);
}

void	sdrplayHandler_v3::stopReader	() {
stopRequest r;
        if (!receiverRuns. load ())
           return;
        messageHandler (&r);
}
//

void	sdrplayHandler_v3::resetBuffer	() {
	_I_Buffer -> FlushRingBuffer();
}

int16_t	sdrplayHandler_v3::bitDepth	() {
	return nrBits;
}

QString	sdrplayHandler_v3::deviceName	() {
	return deviceModel;
}

///////////////////////////////////////////////////////////////////////////
//	Handling the GUI
//////////////////////////////////////////////////////////////////////
//
//	Since the daemon is not threadproof, we have to package the
//	actual interface into its own thread.
//	Communication with that thread is synchronous!
//

void    sdrplayHandler_v3::set_nrBits (int b) {
        nrBits  = b;
        denominator = nrBits == 12 ? 2048 : 4096;
}

void	sdrplayHandler_v3::set_lnabounds(int low, int high) {
	lnaGainSetting	-> setRange (low, high);
}

void	sdrplayHandler_v3::set_deviceName (const QString& s) {
	deviceLabel	-> setText (s);
}

void	sdrplayHandler_v3::set_serial	(const QString& s) {
	serialNumber	-> setText (s);
}

void	sdrplayHandler_v3::set_apiVersion (float version) {
QString v = QString::number (version, 'r', 2);
	api_version	-> display (v);
}

void    sdrplayHandler_v3::show_lnaGain (int g) {
        lnaGRdBDisplay  -> display (g);
}

void	sdrplayHandler_v3::set_ifgainReduction	(int GRdB) {
GRdBRequest r (GRdB);

	if (!receiverRuns. load ())
           return;
        messageHandler (&r);
}

void	sdrplayHandler_v3::set_lnagainReduction (int lnaState) {
lnaRequest r (lnaState);

	if ((lnaGainSetting -> minimum () > lnaState) ||
	    (lnaGainSetting -> maximum () < lnaState))
	   return;
	if (!receiverRuns. load ())
           return;
        messageHandler (&r);
}

void	sdrplayHandler_v3::set_agcControl (int dummy) {
bool    agcMode = agcControl -> isChecked ();
agcRequest r (agcMode, 30);
	(void)dummy;
        messageHandler (&r);
	if (agcMode) {
           GRdBSelector         -> hide ();
           gainsliderLabel      -> hide ();
	}
	else {
	   GRdBRequest r2 (GRdBSelector -> value ());
	   GRdBSelector		-> show ();
	   gainsliderLabel	-> show ();
	   messageHandler  (&r2);
	}
}

void	sdrplayHandler_v3::set_ppmControl (int ppm) {
ppmRequest r (ppm);
        messageHandler (&r);
}

void	sdrplayHandler_v3::set_biasT (int v) {
biasT_Request r (biasT_selector -> isChecked () ? 1 : 0);

	messageHandler (&r);
	sdrplaySettings -> setValue ("biasT_selector",
	                              biasT_selector -> isChecked () ? 1 : 0);
}

void	sdrplayHandler_v3::set_selectAntenna	(const QString &s) {
	messageHandler (new antennaRequest (s == "Antenna A" ? 'A' :
	                                    s == "Antenna B" ? 'B' : 'C'));
}

//
////////////////////////////////////////////////////////////////////////
//	showing data
////////////////////////////////////////////////////////////////////////
void	sdrplayHandler_v3::set_antennaSelect (int n) {
	if (n > 0) {
	   antennaSelector      -> addItem ("Antenna B");
	   if (n > 1)
	      antennaSelector      -> addItem ("Antenna C");
           antennaSelector              -> show ();
        }
        else 
           antennaSelector              -> hide ();
}

void	sdrplayHandler_v3::show_tunerSelector	(bool b) {
	if (b)
	   tunerSelector	-> show	();
	else
	   tunerSelector	-> hide	();
}

//
///////////////////////////////////////////////////////////////////////
//	the real controller starts here
///////////////////////////////////////////////////////////////////////

bool    sdrplayHandler_v3::messageHandler (generalCommand *r) {
        server_queue. push (r);
	serverjobs. release (1);
	while (!r -> waiter. tryAcquire (1, 1000))
	   if (!threadRuns. load ())
	      return false;
	return true;
}
//
//	since the bw for the device is set to 200 and we decimate
//	to 192, we just plain decimate without filtering
static
void    StreamACallback (short *xi, short *xq,
                         sdrplay_api_StreamCbParamsT *params,
                         unsigned int numSamples,
	                 unsigned int reset,
                         void *cbContext) {
sdrplayHandler_v3 *p	= static_cast<sdrplayHandler_v3 *> (cbContext);
std::complex<float> localBuf [numSamples];
float	denominator	= (float) (p -> denominator);
	(void)params;
	if (reset)
	   return;
	if (!p -> receiverRuns. load ())
	   return;

	int	cnt = 0;
	localBuf [0] = std::complex<float> (0, 0);
	static int teller = 0;
	for (int i = 0; i <  (int)numSamples; i ++) {
	   std::complex<float> symb =
	                std::complex<float> (xi [i] / denominator,
	                                     xq [i] / denominator);
	   localBuf [cnt] += symb;
	   teller ++;
	   if (teller >= 12) {
	      teller = 0;
	      localBuf [cnt] /= 4.0f;
	      cnt ++;
	   }
	   
	}
	p -> _I_Buffer -> putDataIntoBuffer (localBuf, cnt);
	if (p -> _I_Buffer -> GetRingBufferReadAvailable () > 1920)
           p -> newdataAvailable (1920);

}

static
void	StreamBCallback (short *xi, short *xq,
                         sdrplay_api_StreamCbParamsT *params,
                         unsigned int numSamples, unsigned int reset,
                         void *cbContext) {
	(void)xi; (void)xq; (void)params; (void)cbContext;
        if (reset)
           printf ("sdrplay_api_StreamBCallback: numSamples=%d\n", numSamples);
}

static
void	EventCallback (sdrplay_api_EventT eventId,
                       sdrplay_api_TunerSelectT tuner,
                       sdrplay_api_EventParamsT *params,
                       void *cbContext) {
sdrplayHandler_v3 *p	= static_cast<sdrplayHandler_v3 *> (cbContext);
	(void)tuner;
	p -> theGain	= params -> gainParams. currGain;
	switch (eventId) {
	   case sdrplay_api_GainChange:
	      break;

	   case sdrplay_api_PowerOverloadChange:
	      p -> update_PowerOverload (params);
	      break;

	   default:
	      fprintf (stderr, "event %d\n", eventId);
	      break;
	}
}

void	sdrplayHandler_v3::
	         update_PowerOverload (sdrplay_api_EventParamsT *params) {
	sdrplay_api_Update (chosenDevice -> dev,
	                    chosenDevice -> tuner,
	                    sdrplay_api_Update_Ctrl_OverloadMsgAck,
	                    sdrplay_api_Update_Ext1_None);
	if (params -> powerOverloadParams.powerOverloadChangeType ==
	                                    sdrplay_api_Overload_Detected) {
//	   fprintf (stderr, "Qt-DAB sdrplay_api_Overload_Detected");
	}
	else {
//	   fprintf (stderr, "Qt-DAB sdrplay_api_Overload Corrected");
	}
}

void	sdrplayHandler_v3::run		() {
sdrplay_api_ErrT        err;
sdrplay_api_DeviceT     devs [6];
uint32_t                ndev;

        threadRuns. store (false);
	receiverRuns. store (false);

	chosenDevice		= nullptr;

	connect (this, SIGNAL (set_serial_signal (const QString &)),
	         this, SLOT (set_serial (const QString &)));
	connect (this, SIGNAL (set_apiVersion_signal (float)),
	         this, SLOT (set_apiVersion (float)));

	denominator		= 2048;		// default
	nrBits			= 12;		// default

	Handle			= fetchLibrary ();
	if (Handle == nullptr) {
	   failFlag. store (true);
	   errorCode	= 1;
	   return;
	}

//	load the functions
	bool success	= loadFunctions ();
	if (!success) {
	   failFlag. store (true);
	   releaseLibrary ();
	   errorCode	= 2;
	   return;
        }
	fprintf (stderr, "functions loaded\n");

//	try to open the API
	err	= sdrplay_api_Open ();
	if (err != sdrplay_api_Success) {
	   fprintf (stderr, "sdrplay_api_Open failed %s\n",
	                          sdrplay_api_GetErrorString (err));
	   failFlag. store (true);
	   releaseLibrary ();
	   errorCode	= 3;
	   return;
	}

	fprintf (stderr, "api opened\n");

//	Check API versions match
        err = sdrplay_api_ApiVersion (&apiVersion);
        if (err  != sdrplay_api_Success) {
           fprintf (stderr, "sdrplay_api_ApiVersion failed %s\n",
                                     sdrplay_api_GetErrorString (err));
	   errorCode	= 4;
	   goto closeAPI;
        }

	if (apiVersion < 3.05) {
           fprintf (stderr, "API versions don't match (local=%.2f dll=%.2f)\n",
                                              SDRPLAY_API_VERSION, apiVersion);
	   errorCode	= 5;
	   goto closeAPI;
	}
	
	fprintf (stderr, "api version %f detected\n", apiVersion);
//
//	lock API while device selection is performed
	sdrplay_api_LockDeviceApi ();
	{  int s	= sizeof (devs) / sizeof (sdrplay_api_DeviceT);
	   err	= sdrplay_api_GetDevices (devs, &ndev, s);
	   if (err != sdrplay_api_Success) {
	      fprintf (stderr, "sdrplay_api_GetDevices failed %s\n",
	                      sdrplay_api_GetErrorString (err));
	      errorCode		= 6;
	      goto unlockDevice_closeAPI;
	   }
	}

	if (ndev == 0) {
	   fprintf (stderr, "no valid device found\n");
	   errorCode	= 7;
	   goto unlockDevice_closeAPI;
	}

	fprintf (stderr, "%d devices detected\n", ndev);
	chosenDevice	= &devs [0];
	err	= sdrplay_api_SelectDevice (chosenDevice);
	if (err != sdrplay_api_Success) {
	   fprintf (stderr, "sdrplay_api_SelectDevice failed %s\n",
	                         sdrplay_api_GetErrorString (err));
	   errorCode	= 8;
	   goto unlockDevice_closeAPI;
	}
//
//	assign callback functions
	cbFns. StreamACbFn	= StreamACallback;
	cbFns. StreamBCbFn	= StreamBCallback;
	cbFns. EventCbFn	= EventCallback;

//	we have a device, unlock
	sdrplay_api_UnlockDeviceApi ();
//
	serial		= devs [0]. SerNo;
	hwVersion	= devs [0]. hwVer;
//
	try {
	   switch (hwVersion) {
	      case SDRPLAY_RSPdx_ :
	         theRsp	= new RspDx_handler (this,
	                                     chosenDevice,
	                                     inputRate,
	                                     KHz (14070),
	                                     agcMode,
	                                     lnaState,
	                                     GRdBValue,
	                                     biasT);
	         break;

	      case SDRPLAY_RSP1A_ :
	         theRsp	= new Rsp1A_handler (this,
	                                     chosenDevice,
	                                     inputRate,
	                                     KHz (14070),
	                                     agcMode,
	                                     lnaState,
	                                     GRdBValue,
	                                     biasT);
	         break;

	      case SDRPLAY_RSP2_ :
	         theRsp	= new RspII_handler (this,
	                                     chosenDevice,
	                                     inputRate,
	                                     KHz (14070),
	                                     agcMode,
	                                     lnaState,
	                                     GRdBValue,
	                                     biasT);
	         break;

	      case SDRPLAY_RSPduo_ :
	         theRsp	= new RspDuo_handler (this,
	                                     chosenDevice,
	                                     inputRate,
	                                     KHz (14070),
	                                     agcMode,
	                                     lnaState,
	                                     GRdBValue,
	                                     biasT);
	         break;

	      default:
	         theRsp	= new Rsp_device (this,
	                                  chosenDevice,
	                                  2112000,
	                                  KHz (14070),
	                                  agcMode,
	                                  lnaState,
	                                  GRdBValue,
	                                  biasT);
	         break;
	   }
	} catch (int e) {
	   goto closeAPI;
	}

	set_serial_signal       (serial);
        set_apiVersion_signal   (apiVersion);

	threadRuns. store (true);       // it seems we can do some work
	successFlag. store (true);
	while (threadRuns. load ()) {
	   while (!serverjobs. tryAcquire (1, 1000))
	   if (!threadRuns. load ())
	      goto normal_exit;

//	here we could assert that the server_queue is not empty
//	Note that we emulate synchronous calling, so
//	we signal the caller when we are done
	   switch (server_queue. front () -> cmd) {
	      case RESTART_REQUEST: {
	         restartRequest *p = (restartRequest *)(server_queue. front ());
	         server_queue. pop ();
	         p -> result = theRsp -> restart (p -> freq);
	         receiverRuns. store (true);
	         p -> waiter. release (1);
	         break;
	      }
	       
	      case STOP_REQUEST: {
	         stopRequest *p = (stopRequest *)(server_queue. front ());
	         server_queue. pop ();
	         receiverRuns. store (false);
	         p -> waiter. release (1);
	         break;
	      }
	       
	      case AGC_REQUEST: {
	         agcRequest *p = 
	                    (agcRequest *)(server_queue. front ());
	         server_queue. pop ();
	         p -> result = theRsp -> set_agc (-p -> setPoint, p -> agcMode);
	         p -> waiter. release (1);
	         break;
	      }

	      case GRDB_REQUEST: {
	         GRdBRequest *p =  (GRdBRequest *)(server_queue. front ());
	         server_queue. pop ();
	         p -> result = theRsp -> set_GRdB (p -> GRdBValue);
                 p -> waiter. release (1);
	         break;
	      }

	      case PPM_REQUEST: {
	         ppmRequest *p = (ppmRequest *)(server_queue. front ());
	         server_queue. pop ();
	         p -> result = theRsp -> set_ppm (p -> ppmValue);
	         p -> waiter. release (1);
	         break;
	      }

	      case LNA_REQUEST: {
	         lnaRequest *p = (lnaRequest *)(server_queue. front ());
	         server_queue. pop ();
	         p -> result = theRsp -> set_lna (p -> lnaState);
                 p -> waiter. release (1);
	         break;
	      }

	      case ANTENNASELECT_REQUEST: {
	         antennaRequest *p = (antennaRequest *)(server_queue. front ());
	         server_queue. pop ();
	         p -> result = theRsp -> set_antenna (p -> antenna);
                 p -> waiter. release (1);
	         break;
	      }

	      case BIAS_T_REQUEST: {
	         biasT_Request *p = (biasT_Request *)(server_queue. front ());
	         server_queue. pop ();
	         p -> result = theRsp -> set_biasT (p -> checked);
                 p -> waiter. release (1);
	         break;
	      }
	
	      default:		// cannot happen
	         fprintf (stderr, "Helemaal fout\n");
	         break;
	   }
	}


normal_exit:
	err = sdrplay_api_Uninit	(chosenDevice -> dev);
	if (err != sdrplay_api_Success) 
	   fprintf (stderr, "sdrplay_api_Uninit failed %s\n",
	                          sdrplay_api_GetErrorString (err));

	err = sdrplay_api_ReleaseDevice	(chosenDevice);
	if (err != sdrplay_api_Success) 
	   fprintf (stderr, "sdrplay_api_ReleaseDevice failed %s\n",
	                          sdrplay_api_GetErrorString (err));

//	sdrplay_api_UnlockDeviceApi	(); ??
        sdrplay_api_Close               ();
	if (err != sdrplay_api_Success) 
	   fprintf (stderr, "sdrplay_api_Close failed %s\n",
	                          sdrplay_api_GetErrorString (err));

	releaseLibrary			();
	fprintf (stderr, "library released, ready to stop thread\n");
	msleep (200);
	return;

unlockDevice_closeAPI:
	sdrplay_api_UnlockDeviceApi	();
closeAPI:	
	failFlag. store (true);
	sdrplay_api_ReleaseDevice       (chosenDevice);
        sdrplay_api_Close               ();
	releaseLibrary	();
	fprintf (stderr, "De taak is gestopt\n");
}

/////////////////////////////////////////////////////////////////////////////
//	handling the library
/////////////////////////////////////////////////////////////////////////////

HINSTANCE	sdrplayHandler_v3::fetchLibrary () {
HINSTANCE	Handle	= nullptr;
#ifdef	__MINGW32__
HKEY APIkey;
wchar_t APIkeyValue [256];
ULONG APIkeyValue_length = 255;

	wchar_t *libname = (wchar_t *)L"sdrplay_api.dll";
	Handle	= LoadLibrary (libname);
	if (Handle == nullptr) {
	   if (RegOpenKey (HKEY_LOCAL_MACHINE,
	                   TEXT("Software\\MiricsSDR\\API"),
	                   &APIkey) != ERROR_SUCCESS) {
              fprintf (stderr,
	           "failed to locate API registry entry, error = %d\n",
	           (int)GetLastError());
	      return nullptr;
	   }

	   RegQueryValueEx (APIkey,
	                    (wchar_t *)L"Install_Dir",
	                    nullptr,
	                    nullptr,
	                    (LPBYTE)&APIkeyValue,
	                    (LPDWORD)&APIkeyValue_length);
//	Ok, make explicit it is in the 32/64 bits section
	   wchar_t *x =
#ifndef __BITS64__
	        wcscat (APIkeyValue, (wchar_t *)L"\\x86\\sdrplay_api.dll");
#else
	        wcscat (APIkeyValue, (wchar_t *)L"\\x64\\sdrplay_api.dll");
#endif
	   RegCloseKey(APIkey);

	   Handle	= LoadLibrary (x);
	   if (Handle == nullptr) {
	      const wchar_t *y =
	              L"C:\\Program Files\\SDRplay\\API\\x86\\sdrplay_api.dll";
	      Handle	= LoadLibrary (y);
	   }
	   if (Handle == nullptr) {
	      fprintf (stderr, "Failed to open sdrplay_api.dll\n");
	      return nullptr;
	   }
	}
#else
	Handle		= dlopen ("libusb-1.0.so", RTLD_NOW | RTLD_GLOBAL);
	Handle		= dlopen ("libsdrplay_api.so", RTLD_NOW);
	if (Handle == nullptr) {
	   fprintf (stderr, "error report %s\n", dlerror());
	   return nullptr;
	}
#endif
	return Handle;
}

void	sdrplayHandler_v3::releaseLibrary () {
#ifdef __MINGW32__
        FreeLibrary (Handle);
#else
	dlclose (Handle);
#endif
}

bool	sdrplayHandler_v3::loadFunctions () {
	sdrplay_api_Open	= (sdrplay_api_Open_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_Open");
	if ((void *)sdrplay_api_Open == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_Open\n");
	   return false;
	}

	sdrplay_api_Close	= (sdrplay_api_Close_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_Close");
	if (sdrplay_api_Close == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_Close\n");
	   return false;
	}

	sdrplay_api_ApiVersion	= (sdrplay_api_ApiVersion_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_ApiVersion");
	if (sdrplay_api_ApiVersion == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_ApiVersion\n");
	   return false;
	}

	sdrplay_api_LockDeviceApi	= (sdrplay_api_LockDeviceApi_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_LockDeviceApi");
	if (sdrplay_api_LockDeviceApi == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_LockdeviceApi\n");
	   return false;
	}

	sdrplay_api_UnlockDeviceApi	= (sdrplay_api_UnlockDeviceApi_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_UnlockDeviceApi");
	if (sdrplay_api_UnlockDeviceApi == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_UnlockdeviceApi\n");
	   return false;
	}

	sdrplay_api_GetDevices		= (sdrplay_api_GetDevices_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_GetDevices");
	if (sdrplay_api_GetDevices == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_GetDevices\n");
	   return false;
	}

	sdrplay_api_SelectDevice	= (sdrplay_api_SelectDevice_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_SelectDevice");
	if (sdrplay_api_SelectDevice == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_SelectDevice\n");
	   return false;
	}

	sdrplay_api_ReleaseDevice	= (sdrplay_api_ReleaseDevice_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_ReleaseDevice");
	if (sdrplay_api_ReleaseDevice == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_ReleaseDevice\n");
	   return false;
	}

	sdrplay_api_GetErrorString	= (sdrplay_api_GetErrorString_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_GetErrorString");
	if (sdrplay_api_GetErrorString == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_GetErrorString\n");
	   return false;
	}

	sdrplay_api_GetLastError	= (sdrplay_api_GetLastError_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_GetLastError");
	if (sdrplay_api_GetLastError == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_GetLastError\n");
	   return false;
	}

	sdrplay_api_DebugEnable		= (sdrplay_api_DebugEnable_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_DebugEnable");
	if (sdrplay_api_DebugEnable == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_DebugEnable\n");
	   return false;
	}

	sdrplay_api_GetDeviceParams	= (sdrplay_api_GetDeviceParams_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_GetDeviceParams");
	if (sdrplay_api_GetDeviceParams == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_GetDeviceParams\n");
	   return false;
	}

	sdrplay_api_Init		= (sdrplay_api_Init_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_Init");
	if (sdrplay_api_Init == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_Init\n");
	   return false;
	}

	sdrplay_api_Uninit		= (sdrplay_api_Uninit_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_Uninit");
	if (sdrplay_api_Uninit == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_Uninit\n");
	   return false;
	}

	sdrplay_api_Update		= (sdrplay_api_Update_t)
	                 GETPROCADDRESS (Handle, "sdrplay_api_Update");
	if (sdrplay_api_Update == nullptr) {
	   fprintf (stderr, "Could not find sdrplay_api_Update\n");
	   return false;
	}

	return true;
}

void	sdrplayHandler_v3::newdataAvailable (int n) {
	dataAvailable (n);
}
