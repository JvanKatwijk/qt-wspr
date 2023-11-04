#
/*
 *    Copyright (C) 2022
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the wspr plugin
 *
 *    wspr plugin is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    wspr plugin is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with wspr plugin; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	"identity-dialog.h"


identityDialog::identityDialog (QSettings *wsprSettings, QWidget *parent):
	                                        QDialog (parent) {
	this	-> wsprSettings	= wsprSettings;
	callsignLabel	= new QLabel (tr("callsign"));
	gridLabel	= new QLabel (tr("grid"));
	antennaLabel	= new QLabel (tr("antenna"));

	callsign	= new QLineEdit;
	homeGrid	= new QLineEdit;
	antenna		= new QLineEdit;

	buttonBox	= new QDialogButtonBox (QDialogButtonBox::Ok |
	                                        QDialogButtonBox::Cancel);
	connect (buttonBox, &QDialogButtonBox::accepted,
	                                this, &identityDialog::verify);
	connect (buttonBox, &QDialogButtonBox::rejected,
	                                this, &identityDialog::reject);
	QGridLayout *mainLayout = new QGridLayout;
	mainLayout	-> addWidget (callsignLabel, 0, 0);
	mainLayout	-> addWidget (callsign, 0, 1);
	mainLayout	-> addWidget (gridLabel, 1, 0);
	mainLayout	-> addWidget (homeGrid, 1, 1);
	mainLayout	-> addWidget (antennaLabel, 2, 0);
	mainLayout	-> addWidget (antenna, 2, 1);
	mainLayout	-> addWidget (buttonBox, 3, 0);
	setLayout (mainLayout);
	setWindowTitle ("your data");
	show ();
}

identityDialog::~identityDialog	() {}

void	identityDialog::verify	() {
	if (!callsign -> text (). isEmpty () &&
	    !homeGrid -> text (). isEmpty () &&
	    !antenna -> text (). isEmpty ()) {
	   wsprSettings	-> setValue ("homeCall", callsign -> text ());
	   wsprSettings	-> setValue ("homeGrid", homeGrid -> text ());
	   wsprSettings	-> setValue ("antenna",  antenna -> text ());
	   accept ();
	   return;
	}

	QMessageBox::StandardButton answer;
	answer = QMessageBox::warning (this, tr ("incomplete form"),
	            tr ("The form is not filled in completely\n"
	                 "Do you want to discard it?"),
	                      QMessageBox::Yes |QMessageBox::No);
	if (answer == QMessageBox::Yes)
	   reject ();
}

void	identityDialog::reject	() {
	accept ();
}

