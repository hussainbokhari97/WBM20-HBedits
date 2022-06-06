/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDTP2M_TempRiver.c

wil.wollheim@unh.edu

Calculate the temperature in runoff from the local grid cell.  Weight groundwater and surface water temperatures.

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInSurfCore_RunoffID = MFUnset;
static int _MDInBaseFlowID        = MFUnset;
static int _MDInWTempSurfRunoffID = MFUnset;
static int _MDInWTempGrdWaterID   = MFUnset;

// Output
static int _MDOutWTempRunoffID          = MFUnset;

static void _MDWTempRunoff (int itemID) {
	float surfRunoffT;
	float baseFlowT;
	float surfaceRO;
	float baseFlow;
	float TemperatureRO;

	surfaceRO   = MFVarGetFloat (_MDInSurfCore_RunoffID, itemID, 0.0);
	baseFlow    = MFVarGetFloat (_MDInBaseFlowID,        itemID, 0.0);
	surfRunoffT = MFVarGetFloat (_MDInWTempSurfRunoffID, itemID, 0.0);
	baseFlowT   = MFVarGetFloat (_MDInWTempGrdWaterID,   itemID, 0.0);

	surfaceRO = MDMaximum (0, surfaceRO);
	baseFlow  = MDMaximum (0, baseFlow);

	TemperatureRO = surfaceRO + baseFlow > 0 ? MDMaximum ((((surfaceRO * surfRunoffT) + (baseFlow * baseFlowT)) / (surfaceRO + baseFlow)),0.0)
	                                         : MDMaximum (surfRunoffT, baseFlowT);
	MFVarSetFloat(_MDOutWTempRunoffID,itemID,TemperatureRO);
}

int MDTP2M_WTempRunoffDef () {

	if (_MDOutWTempRunoffID != MFUnset) return (_MDOutWTempRunoffID);

	MFDefEntering ("Runoff temperature");

	if (((_MDInSurfCore_RunoffID = MDCore_RainSurfRunoffDef())  == CMfailed) ||
        ((_MDInBaseFlowID        = MDCore_BaseFlowDef())        == CMfailed) ||
        ((_MDInWTempSurfRunoffID = MDTP2M_WTempSurfRunoffDef()) == CMfailed) ||
        ((_MDInWTempGrdWaterID   = MDTP2M_WTempGrdWaterDef())   == CMfailed) ||
        ((_MDOutWTempRunoffID    = MFVarGetID (MDVarTP2M_WTempRunoff, "degC", MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempRunoff) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Runoff temperature");
	return (_MDOutWTempRunoffID);
}
