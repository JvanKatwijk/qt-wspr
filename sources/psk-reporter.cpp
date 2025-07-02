#
/*
 *    Copyright (C) 2022
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the wspr decoder
 *
 *    wspr decoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    wspr decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with wspr decoder if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	"psk-reporter.h"

#include	<stdio.h>
#include	<stdlib.h>
#include	<stdint.h>
#include	<string.h>
#include	<unistd.h>
#include	<time.h>
#include	<QMessageBox>
#include	<QSettings>
#include	"radio.h"

const char name [] = "report.pskreporter.info";
const char soft [] = "qt_wspr";

uint8_t header[] = {
//
//	kop.modified dynamically
    0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	pattern for receiver callsign
    0x00, 0x03, 0x00, 0x2C, 0x99, 0x92, 0x00, 0x04,
    0x00, 0x00,
    0x80, 0x02, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x04, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x09, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x00, 0x00,
//	pattern for sender callsign
    0x00, 0x02, 0x00, 0x44, 0x99, 0x93, 0x00, 0x08,
    0x80, 0x01, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x05, 0x00, 0x04, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x06, 0x00, 0x01, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x07, 0x00, 0x01, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x0A, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x0B, 0x00, 0x01, 0x00, 0x00, 0x76, 0x8F,
    0x80, 0x03, 0xFF, 0xFF, 0x00, 0x00, 0x76, 0x8F,
    0x00, 0x96, 0x00, 0x04
};

	pskReporter::pskReporter  (RadioInterface *theRadio,
	                           QSettings *wsprSettings) {
struct hostent *host;

	this	-> theRadio	= theRadio;
	this	-> settings	= wsprSettings;
	homeCall	= wsprSettings -> value ("homeCall", ""). toString ();
	homeGrid	= wsprSettings -> value ("homeGrid", ""). toString ();
	antenna		= wsprSettings -> value ("antenna",
	                                        "loop antenna").  toString ();
	this	-> programName	= "qt-wspr";
	sequence		= 1;
	reporterOK		= false;

	if (homeCall == "")
	   throw (19);

	connect (this, SIGNAL (show_status (const QString &)),
	         theRadio, SLOT (show_status (const QString &)));
	fprintf (stderr, "homedata: %s, %s, (%s)\n",
	                           homeCall. toLatin1 (). data (),
	                           homeGrid. toLatin1 (). data (),
	                           antenna.  toLatin1 (). data ());

#ifdef	__MINGW32__
WSADATA wsaData;

	int res = WSAStartup (MAKEWORD (2, 2), &wsaData);
	if (res != NO_ERROR) {
	   fprintf (stderr, "WSAstartup failed\n");
	   throw (21);
	}
	if ((sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
#else
	if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
#endif
	   fprintf (stderr, "Cannot open socket %d.\n", errno);
	   throw (22);
 	}

	if ((host = gethostbyname (name)) == NULL) {
	   QMessageBox::warning (theRadio, "Warning",
                                       "Could not access pskreporter");
	   throw (23);
	}

	memset (&addr, 0, sizeof (addr));
	addr.sin_family = AF_INET;
	memcpy (&addr. sin_addr.s_addr, host -> h_addr, host -> h_length);
//	addr.sin_port = htons (14739);
	addr.sin_port = htons (4739);
//

	offset_for_size			= 2;
	offset_for_transmission_time	= 4;
	offset_for_sequence_number	= 8;
	offset_for_random		= 12;
//
//	The random number is "static" for all  transmissions in this session
	struct timespec ts;
        clock_gettime (CLOCK_REALTIME, &ts);
        srand (ts. tv_nsec / 1000);
        (void)copy_int4 (&header [offset_for_random], rand ());

//	fill the buffer
	memcpy (buffer, header, sizeof (header));
//
	data_offset		= sizeof (header);
	int receiver_start	= data_offset;
	data_offset		+= copy_int2 (&buffer [data_offset], 0x9992);
	int sizeOffset		= data_offset;
	data_offset += 2;	// to be filled in later
	data_offset += copy_char (&buffer [data_offset],
	                                  homeCall. toLatin1 (). data ());
	data_offset += copy_char (&buffer [data_offset],
	                                  homeGrid. toLatin1 (). data ());
	data_offset += copy_char (&buffer [data_offset],
	                                  programName. toLatin1 (). data ());
	data_offset += copy_char (&buffer [data_offset],
	                                  antenna. toLatin1 (). data ());
	
	int padding = (4 - data_offset % 4) % 4;
	memset (&buffer [data_offset], 0, padding);
	data_offset	+= padding;
	copy_int2 (&buffer [sizeOffset], data_offset - receiver_start);

	start_of_transmitters	= data_offset;
	reporterOK	= true;
}

	pskReporter::~pskReporter	() {
	if (reporterOK)
	   close (sock);
}

bool	pskReporter::reporterReady	() {
	return true;
}


int	pskReporter::sendMessages  (std::vector<pskMessage> &messageStack) {
	if (messageStack. size () < 1)
	   return 0;

	fprintf (stderr, "Sending %d messages\n", messageStack. size ());
	data_offset	= start_of_transmitters;
	data_offset	+= copy_int2 (&buffer [data_offset], 0x9993);
	int sizeOffset	= data_offset;
	data_offset	+= 2;	// to be filled in later
	for (int i = 0; i < messageStack. size (); i ++) {
	   pskMessage m = messageStack.at(i);
	   int localBase = data_offset;
	   data_offset += copy_char (&buffer [data_offset],
	                                  m. call. toLatin1 (). data ());
	   data_offset += copy_int4 (&buffer [data_offset],
	                                  m. freq);
	   data_offset += copy_int1 (&buffer [data_offset],
	                                 (int8_t)(m. snr));
	   data_offset += copy_int1 (&buffer [data_offset], 1); // IMD
	   data_offset += copy_char (&buffer [data_offset], "WSPR");
	   data_offset += copy_int1 (&buffer [data_offset], 1); // automatic
	   data_offset += copy_char (&buffer [data_offset],
	                                 m. grid. toLatin1 (). data ());
	   data_offset += copy_int4 (&buffer [data_offset],
	                                 m. seconds);
	}

	int padding = (4 - data_offset % 4) % 4;
	memset (&buffer [data_offset], 0, padding);
	data_offset += padding;
//
//	size of seen transmitters records
	(void)copy_int2 (&buffer [sizeOffset], data_offset - start_of_transmitters);

//	global data
	(void)copy_int2 (&buffer [offset_for_size], data_offset);
	(void)copy_int4 (&buffer [offset_for_transmission_time], time (NULL));
	(void)copy_int4 (&buffer [offset_for_sequence_number], sequence);
	sequence ++;

	int bytesSent = sendto (sock, (const char *)buffer, data_offset, 0,
	                        (struct sockaddr *)&addr, sizeof(addr));
	if (bytesSent <= 0)
	   show_status ("Transfer failed");
	else
	   show_status ("posting OK");
		
	messageStack. resize (0);
	return EXIT_SUCCESS;
}

int	pskReporter::copy_char (uint8_t *pointer, const char *value) {
int8_t size = strlen (value);
	memcpy (pointer, &size, 1);
	pointer += 1;
	memcpy (pointer, value, size);
	return size + 1;
}

int	pskReporter::copy_int1 (uint8_t *pointer, int8_t value) {
	memcpy (pointer, &value, 1);
	return 1;
}

int	pskReporter::copy_int2 (uint8_t *pointer, int16_t value) {
	value = htons (value);
	memcpy (pointer, &value, 2);
	return 2;
}

int	pskReporter::copy_int4 (uint8_t *pointer, int32_t value) {
	value = htonl (value);
	memcpy (pointer, &value, 4);
	return 4;
}

