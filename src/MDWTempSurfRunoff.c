/******************************************************************************

GHAAS Water Balance/Transport Model V2.0
Global Hydrologic Archive and Analysis System
Copyright 1994-2020, UNH - ASRC/CUNY

MDTempSurfRunoff.c

wil.wollheim@unh.edu

EDITED: amiara@ccny.cuny.edu Sep 2016

This module calculates the temperature of surface runoff and infiltration to groundwater
Irrigation inputs are not accounted here.

*******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInAirTempID  = MFUnset;
static int _MDInSnowMeltID = MFUnset;
static int _MDInWetBulbTempID = MFUnset;

// Output
static int _MDOutWTempSurfROID   = MFUnset;

static void _MDWTempSurfRunoff (int itemID) {
	float airT;
	float SurfWatT;
	float wet_b_temp;

    wet_b_temp         = MFVarGetFloat (_MDInWetBulbTempID,     itemID, 0.0);
    airT               = MFVarGetFloat (_MDInAirTempID,         itemID, 0.0);

    SurfWatT = wet_b_temp; // CHANGED TO EQUAL WET BULB --> Feb 22 2019 MIARA
    SurfWatT = (SurfWatT >= airT) ? wet_b_temp : SurfWatT;

    MFVarSetFloat (_MDOutWTempSurfROID, itemID, SurfWatT);
}

int MDWTempSurfRunoffDef () {

	if (_MDOutWTempSurfROID != MFUnset) return (_MDOutWTempSurfROID);

	MFDefEntering ("Surface runoff temperature");

	if (((_MDInSnowMeltID     = MDSPackMeltDef   ()) == CMfailed) ||
        ((_MDInWetBulbTempID  = MDWetBulbTempDef ()) == CMfailed) ||
        ((_MDInAirTempID      = MFVarGetID (MDVarCommon_AirTemperature, "degC", MFInput,  MFState, MFBoundary)) == CMfailed) ||
	    ((_MDOutWTempSurfROID = MFVarGetID (MDVarTP2M_WTempSurfRunoff,  "degC", MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempSurfRunoff) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Surface runoff temperature");
	return (_MDOutWTempSurfROID);
}
