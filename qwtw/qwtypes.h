
#pragma once

#include <string>
#include <functional>
#ifdef QT_DLL
#include <QMetaType>
#endif

#pragma pack(8)
enum SurfDataType {
	sdMesh,
	sdSurf
};

enum JPType {
	jQWT = 1,
	jMarble = 2,
	jQT3D = 3,
	jMathGL = 4,
	jQwSpectrogram = 5
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
	int flag = 0;	///<	&1 -> have time info; &2 -> have point info
	char label[lSize];   ///< line label
};

#pragma pack()

/** A data container for the spectrogramm.

*/
struct SpectrogramInfo {
	int nx = 0;				// number of X points\cells
	int ny = 0;				// number of Y points\cells
	double xmin = 0.0;		// x minimum
	double xmax = 0.0;		// x max
	double ymin = 0.0;		// y min
	double ymax = 0.0;		// y maximum
	double* z = 0;			// spectrogram data itself.  should be a [ny x nx] matrix. 

	/// <summary>
	///  coordinates of the points, for every spectrogramm cell. Like coords of the center of the spectrogramm cells. might be zeros
	/// </summary>	
	double* p = 0;	// [3 x ny x nx] matrix. 
	double* t = 0; // time info for every cell; [ny x nx] matrix. 
};

/**
	info about window position.
*/
struct QWndPos {
	int x;
	int y;
	int w;
	int h;
	int set = 0;	///< if not 0, then change the window pos.
};

#ifdef QT_DLL
Q_DECLARE_METATYPE(SpectrogramInfo);
Q_DECLARE_METATYPE(CBPickerInfo);
Q_DECLARE_METATYPE(QWndPos);
#endif
#pragma pack()

enum ServiceCode {
	qsCallbackStarted = 1,
	qsCallbackFinished = 2
};


typedef  void (*OnUdpCallback)(int, double, double, double, double);

//typedef void (*OnPickerCallback)(const CBPickerInfo&);
typedef std::function<void(const CBPickerInfo&)> OnPickerCallback;

