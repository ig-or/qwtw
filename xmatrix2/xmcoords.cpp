/**
    coordinate conversion routines are here.
    ECEF, WGS84, ENU, NED   etc stuff.

    \file xmcoords.cpp
    \author   Igor Sandler
    \date    nov 2013
    \version 1.0


*/


// =========================================================================
// ============================== Includes =================================

#include "xmcoords.h"


// =========================================================================
// ======================= C functions (nonclass) ==========================

// Translates linear coordinates in ecef to LLA coordinates (longitude, latitude, altitude -> sequence right?)
void ecef2LLA(const double* ecef, double* LLA) {
	double X = ecef[0];
	double Y = ecef[1];
	double Z = ecef[2];

	LLA[1] = atan2(Y, X);

	double p = sqrt(X*X + Y*Y);
	double psi = atan2((Z * XMWGS84::a), (p * XMWGS84::b));
	double sp = sin(psi);
	double cp = cos(psi);

	LLA[0] = atan2((Z + XMWGS84::es*XMWGS84::es*XMWGS84::b*sp*sp*sp), (p - XMWGS84::e*XMWGS84::e*XMWGS84::a*cp*cp*cp));

	double sf = sin(LLA[0]);
	double N = XMWGS84::a / (sqrt(1.0 - XMWGS84::e2*sf*sf));

	LLA[2] = p / cos(LLA[0]) - N;
}

// -------------------------------------------------------------------------

// same as ecef2LLA (more precise ?)
void ecef2LLArtk(const double* ecef, double* LLA) {
	double X = ecef[0];
	double Y = ecef[1];
	double Z = ecef[2];

	LLA[1] = atan2(Y, X);
	int i, nIter = 16;
	bool isLowlat;
	double t0, tOld, tNew = 0., d, p, c = 0., k;
	d = sqrt(X*X + Y*Y);
	isLowlat = (fabs(Z) <= d);
	if(isLowlat) {
		tOld = t0 = Z / d;
		k = 1. / (1. - XMWGS84::e2);
		c = XMWGS84::a * sqrt(k);
		p = c * XMWGS84::e2 / d;
	} else {
		tOld = t0 = d / Z;
		k = 1. - XMWGS84::e2;
		p = -XMWGS84::a * XMWGS84::e2 / Z;
	}
	for(i = 0; i < nIter; i++) {
		tNew = t0 + (p * tOld) / sqrt(k + tOld * tOld);
		if(fabs(tNew - tOld) < DELTA_ZERO_1) {
			break;
		}
		tOld = tNew;
	}
	if(i == nIter) { // error?
		memset(LLA, 0, 3 * sizeof(double));
		return;
	}
	double t2 = tNew * tNew;
	if(isLowlat) {
		LLA[0] = atan(tNew);
		LLA[2] = (d - c / sqrt(k + t2)) * sqrt(1. + t2);
	} else {
		LLA[0] = (Z >= 0 ? pii05 : -pii05) - atan(tNew);
		LLA[2] = (fabs(Z) - XMWGS84::a * k / sqrt(k + t2)) * sqrt(1. + t2);
	}
}

// -------------------------------------------------------------------------

// Translates LLA coordinates to linear ecef coordinates
void LLA2ecef(double* LLA, double* ecef) {
	double sf = sin(LLA[0]);
	double cf = cos(LLA[0]);
	double sL = sin(LLA[1]);
	double cL = cos(LLA[1]);

	double N = XMWGS84::a / (sqrt(1.0 - XMWGS84::e2*sf*sf));
	ecef[0] = (N + LLA[2]) * cf * cL;
	ecef[1] = (N + LLA[2]) * cf * sL;
	ecef[2] = (N * (1.0 - XMWGS84::e2) + LLA[2]) * sf;

}

// -------------------------------------------------------------------------

// Let us get rotation matrix (ECEF->ENU or ENU->ECEF ??) from ecef linear coordinates
void ecef2ENURotation(const double* ecef, DCM& rot) {
	double LLA[3];
	ecef2LLA(ecef, LLA);
	LLA2ENURotation(LLA, rot);
}

// -------------------------------------------------------------------------

// Let us get rotation matrix (ECEF->ENU or ENU->ECEF ??) from LLA coordinates
void LLA2ENURotation(double* LLA, DCM& rot) {

	XMType sl = sin(LLA[1]);  XMType cl = cos(LLA[1]);  
	XMType sf = sin(LLA[0]);  XMType cf = cos(LLA[0]);  

	rot(0, 0) = -sl;		rot(0, 1) = cl;		rot(0, 2) = ZERO;
	rot(1, 0) = -sf*cl;		rot(1, 1) = -sf*sl;		rot(1, 2) = cf;
	rot(2, 0) = cf*cl;		rot(2, 1) = cf*sl;		rot(2, 2) = sf;
}

// -------------------------------------------------------------------------

// Let us get rotation matrix (ECEF->NED or NED->ECEF ??) from ecef coordinates 
void ecef2NEDRotation(double* ecef, DCM& rot) {
	double LLA[3];
	ecef2LLA(ecef, LLA);

	XMType sl = sin(LLA[1]);  XMType cl = cos(LLA[1]);  
	XMType sf = sin(LLA[0]);  XMType cf = cos(LLA[0]);  


	rot(0, 0) = -sf*cl;		rot(0, 1) = -sf*sl;		rot(0, 2) = cf;
	rot(1, 0) = -sl;		rot(1, 1) = cl;			rot(1, 2) = ZERO;
	rot(2, 0) = -cf*cl;		rot(2, 1) = -cf*sl;		rot(2, 2) = -sf;
}

// -------------------------------------------------------------------------

// Translates coordinates of some vector from ENU to NED
void ENU2NED(const Vector3& enu, Vector3& ned) {
	ned.x[0] = enu.x[1];
	ned.x[1] = enu.x[0];
	ned.x[2] = -enu.x[2];
}




// =========================================================================
// ====================== ENUframe class functions =========================

ENUframe::ENUframe() {
	located = false;
}

// -------------------------------------------------------------------------

ENUframe::ENUframe(const double* pos) {
	setOrigin(pos);
}

// -------------------------------------------------------------------------

void ENUframe::setOrigin(const double* pos) {
	origin[0] = pos[0]; origin[1] = pos[1]; origin[2] = pos[2]; 
	ecef2LLA(origin, originLLA);

	LLA2ENURotation(originLLA, ecef2enuRotation);
	enu2ecefRotation = !ecef2enuRotation;

	double b2 = originLLA[0] * 0.5;
	double l2 = originLLA[1] * 0.5;
	
	ecef2enuQua(0) = cos(b2 + l2) + sin(b2 - l2);
	ecef2enuQua(1) = cos(b2 - l2) - sin(b2 + l2);
	ecef2enuQua(2) = cos(b2 + l2) - sin(b2 - l2);
	ecef2enuQua(3) = cos(b2 - l2) + sin(b2 + l2);
	ecef2enuQua.dmul(0.5);

	enu2ecefQua = ecef2enuQua.conj();

	located = true;
}

// -------------------------------------------------------------------------

Vector3	ENUframe::getEnuCoord(double* ecefCoord) const {
	int i;
	double	delta[3];
	Vector3	dx;

	for (i = 0; i < 3; i+= 1) {
		delta[i] = ecefCoord[i] - origin[i];
		dx[i] = delta[i]; // dx may be float or double
	}

	Vector3 ret = ecef2enuRotation * dx;
	return ret;
}

// -------------------------------------------------------------------------

Vector3	ENUframe::getEnuVelocity(float* ecefVelocity) const {
	int i;
	Vector3 velocity;
	for (i = 0; i < 3; i+= 1) {
		velocity[i] = ecefVelocity[i];
	}
	Vector3 ret = ecef2enuRotation * velocity;
	return ret;
}

// -------------------------------------------------------------------------

Vector3	ENUframe::getEnuCoord(const Vector3& ecefCoord)  const {
	int i;
	double	delta[3];
	Vector3	dx;

	for (i = 0; i < 3; i+= 1) {
		delta[i] = ecefCoord[i] - origin[i];
		dx[i] = delta[i]; // dx may be float or double
	}

	Vector3 ret = ecef2enuRotation * dx;
	return ret;
}

// -------------------------------------------------------------------------

Vector3	ENUframe::getEnuVelocity(const Vector3& ecefVelocity) const {
	Vector3 ret = ecef2enuRotation * ecefVelocity;
	return ret;
}

// -------------------------------------------------------------------------

void	ENUframe::getECEFCoord(const Vector3& enuCoord, double* ecefCoord) const {
	Vector3 ret = enu2ecefRotation*enuCoord;
	int i;
	for (i = 0; i < 3; i+= 1) {
		ecefCoord[i] = ret[i] +  origin[i];
	}
}

// -------------------------------------------------------------------------

void	ENUframe::getLLACoord(const Vector3& enuCoord, double* llaCoord) const {
	double ecefCoord[3];
	getECEFCoord(enuCoord, ecefCoord);

	ecef2LLA(ecefCoord, llaCoord);
}

// -------------------------------------------------------------------------

Vector3	ENUframe::getECEFVelocity(const Vector3& enuVelocity) const {
	Vector3 ret = enu2ecefRotation * enuVelocity;
	return ret;
}




// =========================================================================
// ===================== ENUTracker class functions ========================

ENUTracker::ENUTracker() {
	reset();
}

// -------------------------------------------------------------------------

void ENUTracker::reset() {
	lastPosition.empty();
	quickCounter = 0;
}

// -------------------------------------------------------------------------

void ENUTracker::reset(const Vector3& pos) {
	quickCounter = 2500;
#ifndef USE_SINGLE__PRECISION_NUMBERS
	setOrigin(pos.x);
#else
	double x[3];  
	x[0] = pos[0];  x[1] = pos[1];  x[2] = pos[2];
	setOrigin(x);
#endif
	lastPosition = pos;
}

// -------------------------------------------------------------------------

bool ENUTracker::etUpdate(const Vector3& pos) {
	if(quickCounter != 0) {
		quickCounter--;
		return false;
	}
	quickCounter = 2500;
	static const XMType updateDist = 500. * 500.; //  every 500 m
	if((pos - lastPosition).norma2() < updateDist) { //  not yet
		return false;
	}
#ifndef USE_SINGLE__PRECISION_NUMBERS
	setOrigin(pos.x);
#else
	double x[3];
	x[0] = pos[0];  x[1] = pos[1];  x[2] = pos[2];
	setOrigin(x);
#endif	
	lastPosition = pos;
	return true;
}



