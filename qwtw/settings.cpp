

#include "settings.h"
#include "qwproc.h"
#include <boost/property_tree/json_parser.hpp>
#include "boost/filesystem.hpp"
#include <string>

QWSettings   qwSettings;

QWSettings::QWSettings() {
	aMarkerColor_R = 0;
	aMarkerColor_G = 0;
	aMarkerColor_B = 0;
	direction = 0;
}
int QWSettings::qwSave() {
	char folder[512];
	if (!getFolderLocation(folder, 512)) {
		xmprintf(4, "QWSettings::qwSave()   getFolderLocation failed  \n");
		return 1;
	}
	using namespace boost::filesystem;
	namespace pt = boost::property_tree;
	path sf = path(folder) / "settings.json";

	pt::ptree root;
	root.put("aMarkerColor_R", aMarkerColor_R);
	root.put("aMarkerColor_G", aMarkerColor_G);
	root.put("aMarkerColor_B", aMarkerColor_B);
	root.put("direction", direction);
	std::ostringstream oss;
	pt::write_json(oss, root);
	try {
		std::ofstream file;
        file.open(sf.string());
        file << oss.str();
        file.close();
	} catch (const std::exception& ex) {
		xmprintf(0, "QWSettings::qwSave(): cannot save settings file %s (%s)\n", sf.string().c_str(), ex.what());
	}
	xmprintf(5, "QWSettings::qwSave() file saved    \n");
	return 0;
}
int QWSettings::qwLoad() {
	char folder[512];
	if (!getFolderLocation(folder, 512)) {
		xmprintf(4, "QWSettings::qwLoad()   getFolderLocation failed  \n");
		return 1;
	}
	using namespace boost::filesystem;
	namespace pt = boost::property_tree;
	path sf = path(folder) / "settings.json";
	if (!exists(sf)) {
		xmprintf(4, "QWSettings::qwLoad()   cannot find file %s \n", sf.string().c_str());
		return 2;
	}
	pt::ptree root;
	try {
		pt::read_json(sf.string(), root);
	} catch (const std::exception& ex) {
		xmprintf(4, "QWSettings::qwLoad()   error with the settings file (%s) \n", ex.what());
	}

	aMarkerColor_R = root.get<int>("aMarkerColor_R", 0);
	aMarkerColor_G = root.get<int>("aMarkerColor_G", 0);
	aMarkerColor_B = root.get<int>("aMarkerColor_B", 0);
	direction = root.get<int>("direction", 0);

	xmprintf(9, "QWSettings::qwLoad() file loaded    \n");

	return 0;
}

QColor QWSettings::markerColor() const {
	QColor co;
	co.setRed(qwSettings.aMarkerColor_R);
	co.setGreen(qwSettings.aMarkerColor_G);
	co.setBlue(qwSettings.aMarkerColor_B);
	return co;
}



