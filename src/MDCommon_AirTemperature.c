/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDCommon_AirTemperature.c

bfekete@gc.cuny.edu
******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_AirTemperatureDailyID     = MFUnset;
static int _MDInCommon_AirTemperatureMonthlyID   = MFUnset;
static int _MDInCommon_AirTemperatureReferenceID = MFUnset;

static int _MDInCommon_ElevationID               = MFUnset;
static int _MDInCommon_ElevationRefernceID       = MFUnset;
static int _MDInCommon_LapseRateID               = MFUnset;

// Output
static int _MDOutCommon_AirTemperatureID         = MFUnset;

static void _MDAirTemperature (int itemID) {
// Input 
	float airTempDaily, airTempMonthly, airTempRef;
// Output 
	float airTemp; 

	if (MFVarTestMissingVal (_MDInCommon_AirTemperatureDailyID, itemID)) {
		if (MFVarTestMissingVal (_MDInCommon_AirTemperatureMonthlyID, itemID)) {
			if (MFVarTestMissingVal (_MDInCommon_AirTemperatureReferenceID, itemID)) {
				MFVarSetMissingVal  (_MDOutCommon_AirTemperatureID, itemID);
				return;
			}
				airTemp = MFVarGetFloat (_MDInCommon_AirTemperatureReferenceID, itemID, 0.0);		
		}
		else {
			airTempMonthly = MFVarGetFloat (_MDInCommon_AirTemperatureMonthlyID,   itemID, 0.0);
			airTemp        = MFVarGetFloat (_MDInCommon_AirTemperatureReferenceID, itemID, airTempMonthly);		
		}
	}
	else {
		airTempDaily = MFVarGetFloat (_MDInCommon_AirTemperatureDailyID,  itemID, 0.0);
		if (MFVarTestMissingVal (_MDInCommon_AirTemperatureMonthlyID, itemID)) {
			airTemp = airTempDaily;	
		}
		else {
			airTempMonthly = MFVarGetFloat (_MDInCommon_AirTemperatureMonthlyID,   itemID, 0.0);
			airTempRef     = MFVarGetFloat (_MDInCommon_AirTemperatureReferenceID, itemID, airTempMonthly);	
			airTemp = airTempDaily + airTempRef - airTempMonthly;
		}
	}
	MFVarSetFloat (_MDOutCommon_AirTemperatureID, itemID, airTemp);
}

static void _MDAirTemperatureAdjustment (int itemID) {
	float airTemp;
	float elevation, elevRef, lapseRate;

	if (MFVarTestMissingVal (_MDOutCommon_AirTemperatureID, itemID)) { return; }
	airTemp = MFVarGetFloat (_MDOutCommon_AirTemperatureID, itemID, 0.0);
	if (MFVarTestMissingVal (_MDInCommon_ElevationID, itemID) ||
	    MFVarTestMissingVal (_MDInCommon_ElevationRefernceID, itemID)) { return; }

	elevation = MFVarGetFloat (_MDInCommon_ElevationID, itemID, 0.0);
	elevRef   = MFVarGetFloat (_MDInCommon_ElevationRefernceID, itemID, elevation);
	lapseRate = MFVarGetFloat (_MDInCommon_LapseRateID, itemID, 0.0098);
	airTemp = airTemp + (elevRef - elevation) * lapseRate;
	MFVarSetFloat (_MDOutCommon_AirTemperatureID, itemID, airTemp);
}

enum { MDinput, MDdownscale };
enum { MDon,    MDoff};

int MDCommon_AirTemperatureDef () {
	int optID = MDinput, onoffID = MDoff;
	const char *optStr, *optName = MDVarCommon_AirTemperature;
	const char *options [] = { MDInputStr, "downscale", (char *) NULL };
	const char *onoff   [] = { "on", "off", (char *) NULL };

	if (_MDOutCommon_AirTemperatureID != MFUnset) return (_MDOutCommon_AirTemperatureID);

	MFDefEntering ("Air Temperature");
	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options,optStr,true);
	switch (optID) {
		case MDinput: _MDOutCommon_AirTemperatureID     = MFVarGetID (MDVarCommon_AirTemperature,          "degC",   MFInput, MFState, MFBoundary); break;
		case MDdownscale:
			if (((_MDInCommon_AirTemperatureDailyID     = MFVarGetID (MDVarCommon_AirTemperatureDaily,     "degC",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDInCommon_AirTemperatureMonthlyID   = MFVarGetID (MDVarCommon_AirTemperatureMonthly,   "degC",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDInCommon_AirTemperatureReferenceID = MFVarGetID (MDVarCommon_AirTemperatureReference, "degC",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutCommon_AirTemperatureID         = MFVarGetID (MDVarCommon_AirTemperature,          "degC",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDAirTemperature) == CMfailed)) return (CMfailed);
			break;
		default: MFOptionMessage (optName, optStr, options); return (CMfailed);
	}
	if ((optStr = MFOptionGet ("ElevationAdjustment")) != (char *) NULL) onoffID = CMoptLookup (onoff, optStr,true);
	switch (onoffID) {
		case MDon:
			if (((_MDInCommon_ElevationID              = MFVarGetID (MDVarCommon_Elevation,                "m",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
    	        ((_MDInCommon_ElevationRefernceID      = MFVarGetID (MDVarCommon_ElevationReference,       "m",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
        	    ((_MDInCommon_LapseRateID              = MFVarGetID (MDVarCommon_LapseRate,                "degC/m", MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	(MFModelAddFunction (_MDAirTemperatureAdjustment) == CMfailed)) return (CMfailed);
			break;
		case MDoff: break;
		default: MFOptionMessage ("ElevationAdjustment", optStr, onoff); return (CMfailed);
	}
	MFDefLeaving ("Air Temperature");
	return (_MDOutCommon_AirTemperatureID);
}
