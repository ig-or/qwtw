

#pragma once

#include <QColor>

struct QWSettings {

	int aMarkerColor_R;
	int aMarkerColor_G;
	int aMarkerColor_B;
	int direction;
	int pickerDigitsNumber; ///< number of digits to display when press on a plot with an arrow (marker mode)

	QWSettings();
	int qwSave();
	int qwLoad();
	///   update the color
	QColor markerColor() const;

};

extern QWSettings   qwSettings;

