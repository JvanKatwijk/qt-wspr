#
/*
 *    Copyright (C)  2023
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the qt-wspr
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

#ifndef	__SPECTRUM_SCOPE__
#define	__SPECTRUM_SCOPE__
#include	"virtual-scope.h"
#include        <QObject>
#include        <qwt.h>
#include        <qwt_plot.h>
#include        <qwt_plot_curve.h>
#include        <qwt_plot_marker.h>
#include        <qwt_plot_grid.h>
#include        <qwt_color_map.h>
#include        <qwt_plot_picker.h>
#include        <QBrush>
#include        <QTimer>
#include        <stdint.h>

class   QwtLinearColorMap;
class   QwtScaleWidget;


class	spectrumScope: public virtualScope  {
Q_OBJECT
public:
		spectrumScope	(QwtPlot *, int16_t);
		~spectrumScope	();
	void	display		(double *, double *, double, int32_t);
	void	set_bitDepth	(int16_t);
private:
	QwtPlotGrid	*grid;
	QwtPlotCurve	*SpectrumCurve;
	QwtPlotMarker	*Marker;
	uint32_t	IndexforMarker;
	QwtPlotPicker	*lm_picker;
	QwtPlotPicker	*rm_picker;
	QBrush		*ourBrush;
	int16_t		bitDepth;
	int32_t		normalizer;
	float		get_db		(float);
	int32_t		markerValue;
	int32_t		oldmarkerValue;
private slots:
	void	leftMouseClick (const QPointF &);
	void	rightMouseClick (const QPointF &);
};
#endif

