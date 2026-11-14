

#pragma once

#include <QColor>
#include <string>

struct QWSettings {

	int aMarkerColor_R;
	int aMarkerColor_G;
	int aMarkerColor_B;
	int direction;
	int pickerDigitsNumber; ///< number of digits to display when press on a plot with an arrow (marker mode)
	int title_font_size;      ///< plot title font size in pt
	int axis_label_font_size; ///< X/Y axis label font size in pt
	int axis_tick_font_size;  ///< X/Y axis tick number font size in pt
	int legend_font_size;     ///< curve legend font size in pt
	int udp_server_port; ///< port number for UDP server. If UDP server is not working, try to change it in the config file.
	int udp_client_port;  ///< port client for UDP server. If UDP client is not working, try to change it in the config file.
	std::string smip;	///< multicast address
	std::string map_tile_url; ///< slippy-map tile URL template (default: OpenStreetMap)


	QWSettings();
	int qwSave();
	int qwLoad();
	///   update the color
	QColor markerColor() const;

};

extern QWSettings   qwSettings;


