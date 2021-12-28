
#pragma once

#include <string>
#include <functional>

#pragma pack(8)
enum SurfDataType {
	sdMesh,
	sdSurf
};

enum JPType {
	jQWT = 1,
	jMarble = 2,
	jQT3D = 3,
	jMathGL = 4
};

struct MeshInfo {
	int 	xSize;
	int 	ySize;
	double 	xMin;
	double 	xMax;
	double 	yMin;
	double 	yMax;
	double* data; 
	std::string name;
	std::string style;
	SurfDataType sd;
};

struct CBPickerInfo {
	static const int lSize = 64;
	double time = 0.0;
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;

	int index = 0;
	int xx = 0;
	int yy = 0;
	int plotID = 0;
	int lineID = 0;
	int type = 0;
	char label[lSize];
};
#pragma pack()

typedef  void (*OnUdpCallback)(int, double, double, double, double);

//typedef void (*OnPickerCallback)(const CBPickerInfo&);
typedef std::function<void(const CBPickerInfo&)> OnPickerCallback;

