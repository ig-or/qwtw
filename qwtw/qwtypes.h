
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

#pragma pack()

#pragma pack(1)

/// picker info
struct CBPickerInfo {
	static const int lSize = 24;
	double time = 0.0;  ///< closest point
	double x = 0.0;  ///< closest point
	double y = 0.0;  ///< closest point
	double z = 0.0;  ///< closest point

	int index = 0; ///< index of the closest point
	int xx = 0;	///< window coords
	int yy = 0; ///< window coords
	int plotID = 0;
	int lineID = 0;
	int type = 0; 	///< signal source ID
	char label[lSize];   ///< line label
};

#pragma pack()

enum ServiceCode {
	qsCallbackStarted = 1,
	qsCallbackFinished = 2
};


typedef  void (*OnUdpCallback)(int, double, double, double, double);

//typedef void (*OnPickerCallback)(const CBPickerInfo&);
typedef std::function<void(const CBPickerInfo&)> OnPickerCallback;

