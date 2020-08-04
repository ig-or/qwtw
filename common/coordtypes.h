
/**  coordinate types

\file coordtypes.h
\author   Igor Sandler
\date    Fall  2014
\version 1.0

*/



#ifndef COORD_TYPES_H_FILE
#define COORD_TYPES_H_FILE

namespace CoordType {

enum CoordinateType {
    ECEF = 0, // usually in meters (X, Y, Z)
    WGS84,   // lat, lon, altitude [rad, rad, meters]
    CoordTypesCount
};

///    names of those coordinate frames
extern const char*  CoordTypeName[CoordTypesCount];

}


#endif


