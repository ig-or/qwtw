/**
	coordinate conversion routines are here.
	ECEF, WGS84, ENU, NED   etc stuff.

	\file xmcoords.h
	\author   Igor Sandler
	\date    nov 2013
	\version 1.0


*/

#ifndef XMCOORDS_H_FILE
#define XMCOORDS_H_FILE

#include "xmatrix2.h"

/** Useful constants, which define WGS84.

*/
namespace XMWGS84 {
		const double		a	= 6378137.0;
		const double		f	= 1.0 / 298.257223563;
		const double		b	= a * (1.0 - f);
		const double		e	=  0.0818191908426215;		//sqrtf((a*a - b*b) / (a*a));
		const double		e2	=  0.00669437999014132;		// e^2
		const double		es	=  0.0820944379496957;		//sqrtf((a*a - b*b) / (b*b));

		const double		we = 7.2921150e-5;   ///< [rad/second] 360.9856 degrees every day
}

/**  Convert Earth-centered Earth-fixed (ECEF) coordinates to
    geodetic coordinates. 
	This is very different from matlab code (and result is a bit different).
	Becouse matlab does up to 5 iterations, and here we have only 1 iteration.

	But, this is a good approximation up to low earth orbit.

	@param[in] ecef vector in ECEF frame, [m]
	@param[out] LLA    array of geodetic coordinates (latitude, longitude and
		 altitude), LLA.  LLA is in [radians radians meters]. 
*/
void ecef2LLA(const double* ecef, double* LLA);

/**  Convert Earth-centered Earth-fixed (ECEF) coordinates to
geodetic coordinates.
This comes from RTK3 engine code.

@param[in] ecef vector in ECEF frame, [m]
@param[out] LLA    array of geodetic coordinates (latitude, longitude and
altitude), LLA.  LLA is in [radians radians meters].
*/
void ecef2LLArtk(const double* ecef, double* LLA);

/*	@param[out] ecef vector in ECEF frame, [m]
	@param[in] LLA    array of geodetic coordinates (latitude, longitude and
		 altitude), LLA.  LLA is in [radians radians meters]. 
*/
void LLA2ecef(double* LLA, double* ecef);

/** create a rotation matrix (from ecef into ENU frame)
    @param[in] ecef our origin in ECEF frame [meters]
    @param[out] rot rotation matrix
*/
void ecef2ENURotation(const double* ecef, DCM& rot);

/** create a rotation matrix (from ecef into ENU frame)
    @param[in] LLA our origin in ECEF frame, WGS84, latitude, longitude, altitude [radians, radians, meters]
    @param[out] rot rotation matrix
*/

void LLA2ENURotation(double* LLA, DCM& rot);

/** create a rotation matrix (from ecef into NED frame)
@param[in] ecef our origin in ECEF frame [meters]
@param[out] rot rotation matrix
*/
void ecef2NEDRotation(double* ecef, DCM& rot);

/** rotate a point from ENU into NED frame.
    @param[in] enu point in ENU frame
    @param[out] ned point in NED frame
*/
void ENU2NED(const Vector3& enu, Vector3& ned);

/** Small helper class for ECEF <-> ENU rotations.
\code
    Usage:
    double enuOrigin[3];
    enuOrigin = .... put here the origin of our ENU frame (it's coordinates in ECEF frame, [m])
    ENUframe fa; //1.  create an instance of this class:
    fa.setOrigin(enuOrigin); // 2. set up the origin
    // 3. use all functions

\endcode

    \class ENUframe
*/
class ENUframe {
public:
	ENUframe();
	ENUframe(const double* pos);

	/** set up an ENU origin.
		@param[in] pos origin in ECEF frame [m]
	*/
	void setOrigin(const double* pos);

	/** convert ECEF point into ENU point.

	*/
	Vector3	getEnuCoord(double* ecefCoord) const;

	/** convert ECEF velocity into ENU velocity.

	*/
	Vector3	getEnuVelocity(float* ecefVelocity) const;

	/** convert ECEF point into ENU point.

	*/
	Vector3	getEnuCoord(const Vector3& ecefCoord) const;

	/** convert ECEF velocity into ENU velocity.

	*/
	Vector3	getEnuVelocity(const Vector3& ecefVelocity) const;

	/** convert ENU point into ECEF point.

	*/
	void 	getECEFCoord(const Vector3& enuCoord, double* ecefCoord) const;

	/** get WGS84 parameters latitude, longitude, altitude [radians, radians, meters]
	   from ENU coordinate.
	   @param[in] enuCoord point in ENU frame [meters]
	   @param[out] llaCoord WGS84 parameters latitude, longitude, altitude [radians, radians, meters]

	*/
	void		getLLACoord(const Vector3& enuCoord, double* llaCoord) const;
	/** convert ENU velocity into ECEF velocity.

	*/
	Vector3	getECEFVelocity(const Vector3& enuVelocity) const;

	bool located; ///< true if we have valid ecef2enuRotation
	double	originLLA[3];
	Qua  ecef2enuQua, enu2ecefQua;
	DCM	ecef2enuRotation;
	DCM	enu2ecefRotation;
	double	origin[3]; ///< origin in ECEF frame

private:
	
	
	
};

/**  ENU frame, updated

*/
class ENUTracker: public ENUframe {
public:
	ENUTracker();

	/**  update all internal info, if position was changed.
	This function supposed to be called from IMU measurement update
	@param[in] pos new position in ECEF frame [m]
	@return true if position and all matrices were updated
	*/
	bool etUpdate(const Vector3& pos);
	void reset();
	void reset(const Vector3& pos);
private:
	int quickCounter;
	Vector3 lastPosition;
};

#endif

