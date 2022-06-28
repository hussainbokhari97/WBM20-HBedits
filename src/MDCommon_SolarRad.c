/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDCommon_SolarRad.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

static float _MDSRadISC (int doy) {
	float isc;
	isc = 1.0 - .0167 * cos((double) 0.0172 * (double) (doy - 3));
	return (1367.0 / (isc * isc));
}

static float _MDSRadDEC (int doy) {
	double dec;

	dec = sin (6.224111 + 0.017202 * (double) doy);
	dec = 0.39785 * sin (4.868961 + .017203 * (double) doy + 0.033446 * sin (dec));
	return ((float) asin (dec));
}

static float _MDSRadH (float lat,int doy,float dec) {
	float arg, h;
	
   arg = -tan(dec) * tan(lat);
	if (arg > 1.0) h = 0.0;         /* sun stays below horizon */ 
	else if (arg <  -1.0) h = M_PI; /* sun stays above horizon */
	else h = acos(arg);
	return (h);
}

static int _MDOutCommon_SolarRadDayLengthID = MFUnset;

static void _MDCommon_SolarRadDayLength (int itemID) { // daylength fraction of day
// Model
	int doy   = MFDateGetDayOfYear (); // day of the year
	float lat = MFModelGetLatitude (itemID) / 180.0 * M_PI;; // latitude in radians

// Input
// Output
	float dayLength;
// Local
	float dec;

   dec = _MDSRadDEC (doy);

   if (fabs ((double) lat) > M_PI_2) lat = (M_PI_2 - (double) 0.01) * (lat > 0.0 ? 1.0 : -1.0);

	dayLength = _MDSRadH (lat,doy,dec) / M_PI;
	MFVarSetFloat (_MDOutCommon_SolarRadDayLengthID,itemID,dayLength);
}

int MDCommon_SolarRadDayLengthDef () {
	if (_MDOutCommon_SolarRadDayLengthID != MFUnset) return (_MDOutCommon_SolarRadDayLengthID);

	MFDefEntering ("Day length");
	if (((_MDOutCommon_SolarRadDayLengthID   = MFVarGetID (MDVarCore_SolarRadDayLength, "1/d", MFOutput, MFState, false)) == CMfailed) ||
		(MFModelAddFunction(_MDCommon_SolarRadDayLength) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Day length");
	return (_MDOutCommon_SolarRadDayLengthID);
}

static int _MDOutCommon_SolarRadI0HDayID = MFUnset;

static void _MDSRadI0HDay (int itemID) { // daily potential solar radiation from Sellers (1965)
// Model
	int   doy = MFDateGetDayOfYear (); // day of the year
	float lat = MFModelGetLatitude (itemID) / 180.0 * M_PI; // latitude in decimal degrees converted to radian
// Input
// Output
	float i0hDay;
// Local
	float isc, dec, h;

	isc = _MDSRadISC (doy);
	dec = _MDSRadDEC (doy);

	if (fabs ((double) lat) > M_PI_2) lat = (M_PI_2 - (double) 0.01) * (lat > 0.0 ? 1.0 : -1.0);
	h = _MDSRadH (lat,doy,dec);

	i0hDay =	0.000001 * isc * (86400.0 / M_PI) *  (h * sin(lat) * sin(dec) + cos(lat) * cos(dec) * sin(h));
	MFVarSetFloat (_MDOutCommon_SolarRadI0HDayID,itemID,i0hDay);
}

int MDCommon_SolarRadI0HDayDef () {
	if (_MDOutCommon_SolarRadI0HDayID != MFUnset) return (_MDOutCommon_SolarRadI0HDayID);

	MFDefEntering ("I0H Day");
	if (((_MDOutCommon_SolarRadI0HDayID   = MFVarGetID (MDVarCore_SolarRadI0HDay, "MJ/m2", MFOutput, MFState, false)) == CMfailed) ||
        (MFModelAddFunction (_MDSRadI0HDay) == CMfailed)) return (CMfailed);
	MFDefLeaving ("I0H Day");
	return (_MDOutCommon_SolarRadI0HDayID);
}

static int _MDInCommon_CloudCoverID;
static int _MDInCommon_GrossRadID;

static int _MDOutCommon_SolarRadID = MFUnset;

static void _MDSolarRadiationCloud (int itemID) {
// Input
	float cloud    = MFVarGetFloat (_MDInCommon_CloudCoverID, itemID, 0.0); // Cloud cover in percent
	float clearSky = MFVarGetFloat (_MDInCommon_GrossRadID,   itemID, 0.0); // Clear sky radiation in W/m2
// Output
	float solarRad; // Solar radiation W/m2
// Local
 
 	cloud = cloud < 100.0 ? cloud / 100.0 : 1.0;
	solarRad = clearSky * (0.803 - (0.340 * cloud) - (0.458 * (float) pow ((double) cloud,(double) 2.0)));
    MFVarSetFloat (_MDOutCommon_SolarRadID,  itemID, solarRad);
}

static int _MDInCommon_SunShineID;

static void _MDSolarRadiationSun (int itemID) {
// Input
	float sunShine = MFVarGetFloat (_MDInCommon_SunShineID, itemID, 50.0) / 100.0; // Percent of sunny day
	float clearSky = MFVarGetFloat (_MDInCommon_GrossRadID,   itemID,  0.0); // Clear sky/gross radiation W/m2 averaged over the day
// Output
	float solarRad; // Solar radiation W/m2 averaged over the day

	solarRad = solarRad * (0.251 + 0.509 * sunShine);
	MFVarSetFloat (_MDOutCommon_SolarRadID,  itemID, solarRad);
}

enum { MDhelp, MDinput, MDcloud, MDsun };

int MDCommon_SolarRadDef () {
	int optID = MDinput;
	const char *optStr;
	const char *options [] = { MFhelpStr, MFinputStr, "cloud", "sun", (char *) NULL };

	if (_MDOutCommon_SolarRadID != MFUnset) return (_MDOutCommon_SolarRadID);

	MFDefEntering ("Solar Radiation");
	if ((optStr = MFOptionGet (MDVarCore_SolarRadiation)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
	switch (optID) {
		default:
		case MDhelp:  MFOptionMessage (MDVarCore_SolarRadiation, optStr, options); return (CMfailed);
		case MDinput: _MDOutCommon_SolarRadID = MFVarGetID (MDVarCore_SolarRadiation, "W/m^2", MFInput, MFState, MFBoundary); break;
		case MDcloud:
			if (((_MDInCommon_GrossRadID   = MDCommon_GrossRadDef ())  == CMfailed) ||
                ((_MDInCommon_CloudCoverID = MDCommon_CloudCoverDef()) == CMfailed) ||
                ((_MDOutCommon_SolarRadID  = MFVarGetID (MDVarCore_SolarRadiation, "W/m^2", MFOutput,  MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDSolarRadiationCloud) == CMfailed)) return (CMfailed);
			break;
		case MDsun:
			if (((_MDInCommon_GrossRadID   = MDCommon_GrossRadDef()) == CMfailed) ||
                ((_MDInCommon_SunShineID   = MFVarGetID (MDVarCore_SunShine,       "%",     MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutCommon_SolarRadID  = MFVarGetID (MDVarCore_SolarRadiation, "W/m^2", MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDSolarRadiationSun) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Solar Radiation");
	return (_MDOutCommon_SolarRadID);
}
