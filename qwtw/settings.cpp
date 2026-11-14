#include "settings.h"
#include "qwproc.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <filesystem>
#include <string>

QWSettings qwSettings;

QWSettings::QWSettings() {
	aMarkerColor_R = 0;
	aMarkerColor_G = 0;
	aMarkerColor_B = 0;
	direction = 0;
	pickerDigitsNumber = 6;
	udp_server_port = 49562;
	udp_client_port = 49561;
	smip.assign("239.255.0.1");
	map_tile_url.assign("https://tile.openstreetmap.org/{z}/{x}/{y}.png");
}

int QWSettings::qwSave() {
	char folder[512];
	if (!getFolderLocation(folder, 512)) {
		xmprintf(4, "QWSettings::qwSave() getFolderLocation failed\n");
		return 1;
	}

	std::error_code ec;
	std::filesystem::path dir(folder);
	if (!std::filesystem::exists(dir, ec)) {
		std::filesystem::create_directories(dir, ec);
	}
	std::filesystem::path sf = dir / "settings.json";

	QJsonObject root;
	root.insert("aMarkerColor_R", aMarkerColor_R);
	root.insert("aMarkerColor_G", aMarkerColor_G);
	root.insert("aMarkerColor_B", aMarkerColor_B);
	root.insert("direction", direction);
	root.insert("pickerDigitsNumber", pickerDigitsNumber);
	root.insert("udp_server_port", udp_server_port);
	root.insert("udp_client_port", udp_client_port);
	root.insert("smip", QString::fromStdString(smip));
	root.insert("map_tile_url", QString::fromStdString(map_tile_url));

	QFile file(QString::fromStdString(sf.string()));
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		xmprintf(0, "QWSettings::qwSave(): cannot save settings file %s\n", sf.string().c_str());
		return 2;
	}
	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
	file.close();

	xmprintf(5, "QWSettings::qwSave() file saved\n");
	return 0;
}

static int readJsonInt(const QJsonObject& obj, const char* key, int defaultVal) {
	auto it = obj.find(QLatin1String(key));
	if (it == obj.end()) {
		return defaultVal;
	}
	if (it->isDouble()) {
		return it->toInt(defaultVal);
	}
	if (it->isString()) {
		bool ok = false;
		int v = it->toString().toInt(&ok);
		return ok ? v : defaultVal;
	}
	return defaultVal;
}

int QWSettings::qwLoad() {
	char folder[512];
	if (!getFolderLocation(folder, 512)) {
		xmprintf(4, "QWSettings::qwLoad() getFolderLocation failed\n");
		return 1;
	}

	std::error_code ec;
	std::filesystem::path sf = std::filesystem::path(folder) / "settings.json";
	if (!std::filesystem::exists(sf, ec)) {
		xmprintf(4, "QWSettings::qwLoad() cannot find file %s\n", sf.string().c_str());
		return 2;
	}

	QFile file(QString::fromStdString(sf.string()));
	if (!file.open(QIODevice::ReadOnly)) {
		xmprintf(4, "QWSettings::qwLoad() cannot open file %s\n", sf.string().c_str());
		return 2;
	}

	QJsonParseError parseErr;
	QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseErr);
	file.close();
	if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) {
		xmprintf(4, "QWSettings::qwLoad() JSON parse error (%s)\n",
			parseErr.errorString().toStdString().c_str());
		return 3;
	}

	QJsonObject root = doc.object();
	aMarkerColor_R = readJsonInt(root, "aMarkerColor_R", 0);
	aMarkerColor_G = readJsonInt(root, "aMarkerColor_G", 0);
	aMarkerColor_B = readJsonInt(root, "aMarkerColor_B", 0);
	direction = readJsonInt(root, "direction", 0);
	pickerDigitsNumber = readJsonInt(root, "pickerDigitsNumber", 6);
	udp_server_port = readJsonInt(root, "udp_server_port", 49562);
	udp_client_port = readJsonInt(root, "udp_client_port", 49561);

	auto smipIt = root.find(QLatin1String("smip"));
	if (smipIt != root.end() && smipIt->isString()) {
		smip = smipIt->toString().toStdString();
	} else {
		smip = "239.255.0.1";
	}

	auto tileIt = root.find(QLatin1String("map_tile_url"));
	if (tileIt != root.end() && tileIt->isString() && !tileIt->toString().trimmed().isEmpty()) {
		map_tile_url = tileIt->toString().trimmed().toStdString();
	} else {
		map_tile_url = "https://tile.openstreetmap.org/{z}/{x}/{y}.png";
	}

	xmprintf(9, "QWSettings::qwLoad() file loaded\n");
	return 0;
}

QColor QWSettings::markerColor() const {
	QColor co;
	co.setRed(qwSettings.aMarkerColor_R);
	co.setGreen(qwSettings.aMarkerColor_G);
	co.setBlue(qwSettings.aMarkerColor_B);
	return co;
}
