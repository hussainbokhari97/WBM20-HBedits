/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

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
static int _MDInCommon_AdiabaticLapseRateID      = MFUnset;

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
	lapseRate = MFVarGetFloat (_MDInCommon_AdiabaticLapseRateID, itemID, 0.0098);
	airTemp = airTemp + (elevRef - elevation) * lapseRate;
	MFVarSetFloat (_MDOutCommon_AirTemperatureID, itemID, airTemp);
}

enum { MDhelp, MDinput, MDdownscale };

int MDCommon_AirTemperatureDef () {
	int optID = MDinput, switchID = MFoff;
	const char *optStr;
	const char *options [] = { MFhelpStr, MFinputStr, "downscale", (char *) NULL };

	if (_MDOutCommon_AirTemperatureID != MFUnset) return (_MDOutCommon_AirTemperatureID);

	MFDefEntering ("Air Temperature");
	if ((optStr = MFOptionGet (MDVarCommon_AirTemperature)) != (char *) NULL) optID = CMoptLookup (options,optStr,true);
	switch (optID) {
		default:      MFOptionMessage (MDVarCommon_AirTemperature, optStr, options); return (CMfailed);
		case MDhelp:  MFOptionMessage (MDVarCommon_AirTemperature, optStr, options);
		case MDinput: _MDOutCommon_AirTemperatureID = MFVarGetID (MDVarCommon_AirTemperature, "degC",   MFInput,  MFState, MFBoundary); break;
		case MDdownscale:
			if (((_MDInCommon_AirTemperatureDailyID     = MFVarGetID (MDVarCommon_AirTemperatureDaily,     "degC",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDInCommon_AirTemperatureMonthlyID   = MFVarGetID (MDVarCommon_AirTemperatureMonthly,   "degC",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDInCommon_AirTemperatureReferenceID = MFVarGetID (MDVarCommon_AirTemperatureReference, "degC",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutCommon_AirTemperatureID         = MFVarGetID (MDVarCommon_AirTemperature,          "degC",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDAirTemperature) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Air Temperature");

	MFDefEntering ("Elevation Adjustment");
	if ((optStr = MFOptionGet ("ElevationAdjustment")) != (char *) NULL) switchID = CMoptLookup (MFswitchOptions, optStr,true);
	switch (switchID) {
		default:     MFOptionMessage ("ElevationAdjustment", optStr, MFswitchOptions); return (CMfailed);
		case MFhelp: MFOptionMessage ("ElevationAdjustment", optStr, MFswitchOptions);
		case MFoff:  break;
		case MFon:
			MFDefEntering ("Elevation Adjustment");
			if (((_MDInCommon_ElevationID              = MFVarGetID (MDVarCommon_Elevation,                "m",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
    	        ((_MDInCommon_ElevationRefernceID      = MFVarGetID (MDVarCommon_ElevationReference,       "m",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
        	    ((_MDInCommon_AdiabaticLapseRateID     = MFVarGetID (MDVarCommon_LapseRate,                "degC/m", MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	(MFModelAddFunction (_MDAirTemperatureAdjustment) == CMfailed)) return (CMfailed);
			MFDefLeaving ("Elevation Adjustment");
			break;
	}
	MFDefLeaving ("Elevation Adjustment");
	return (_MDOutCommon_AirTemperatureID);
}
