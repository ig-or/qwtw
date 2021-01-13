
#pragma once

#include <string>

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