

#pragma once

#include <QColor>

struct QWSettings {

	int aMarkerColor_R;
	int aMarkerColor_G;
	int aMarkerColor_B;

	QWSettings();
	int qwSave();
	int qwLoad();
	///   update the color
	QColor markerColor() const;

};

extern QWSettings   qwSettings;


