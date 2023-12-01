

#pragma once

#include <QColor>
#include <string>

struct QWSettings {

	int aMarkerColor_R;
	int aMarkerColor_G;
	int aMarkerColor_B;
	int direction;
	int pickerDigitsNumber; ///< number of digits to display when press on a plot with an arrow (marker mode)
	int udp_server_port; ///< port number for UDP server. If UDP server is not working, try to change it in the config file.
	int udp_client_port;  ///< port client for UDP server. If UDP client is not working, try to change it in the config file.
	std::string smip;	///< multicast address


	QWSettings();
	int qwSave();
	int qwLoad();
	///   update the color
	QColor markerColor() const;

};

extern QWSettings   qwSettings;


