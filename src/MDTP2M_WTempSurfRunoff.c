/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDTP2M_TempSurfRunoff.c

wil.wollheim@unh.edu

EDITED: amiara@ccny.cuny.edu Sep 2016
EDITED: ariel.miara@nrel.gov Feb11 2021

This module calculates the temperature of surface runoff and infiltration to groundwater
Irrigation inputs are not accounted here.

*******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_AirTemperatureID = MFUnset;

// Output
static int _MDOutWTempSurfROID          = MFUnset;

static void _MDWTempSurfRunoff (int itemID) {
    MFVarSetFloat (_MDOutWTempSurfROID, itemID, MDMaximum (MFVarGetFloat (_MDInCommon_AirTemperatureID, itemID, 0.0) - 1.5, 0.0));
}

int MDTP2M_WTempSurfRunoffDef () {

	if (_MDOutWTempSurfROID != MFUnset) return (_MDOutWTempSurfROID);

	MFDefEntering ("Surface runoff temperature");
	if (((_MDInCommon_AirTemperatureID = MDCommon_AirTemperatureDef ()) == CMfailed) ||
        ((_MDOutWTempSurfROID          = MFVarGetID (MDVarTP2M_WTempSurfRunoff,  "degC", MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempSurfRunoff) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Surface runoff temperature");
	return (_MDOutWTempSurfROID);
}
