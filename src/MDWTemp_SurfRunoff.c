/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDTP2M_TempSurfRunoff.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_AirTemperatureID = MFUnset;

// Output
static int _MDOutWTempSurfROID          = MFUnset;

static void _MDWTempSurfRunoff (int itemID) {
	float airTemp = MFVarGetFloat (_MDInCommon_AirTemperatureID, itemID, 0.0); // Air temperature degC
	float surfRunoffTemp;

	surfRunoffTemp = airTemp > 1.5 ? airTemp - 1.5 : (airTemp > 0.0 ? airTemp : 0.0);
    MFVarSetFloat (_MDOutWTempSurfROID, itemID, surfRunoffTemp);
}

int MDWTemp_SurfRunoffDef () {

	if (_MDOutWTempSurfROID != MFUnset) return (_MDOutWTempSurfROID);

	MFDefEntering ("Surface runoff temperature");
	if (((_MDInCommon_AirTemperatureID = MDCommon_AirTemperatureDef ()) == CMfailed) ||
        ((_MDOutWTempSurfROID          = MFVarGetID (MDVarWTemp_SurfRunoff,  "degC", MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempSurfRunoff) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Surface runoff temperature");
	return (_MDOutWTempSurfROID);
}
