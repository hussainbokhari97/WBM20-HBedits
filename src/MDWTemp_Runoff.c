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
static int _MDInCore_SurfRunoffID  = MFUnset;
static int _MDInCore_BaseFlowID    = MFUnset;
static int _MDInWTemp_SurfRunoffID = MFUnset;
static int _MDInWTemp_GrdWaterID   = MFUnset;

// Output
static int _MDOutWTemp_RunoffID    = MFUnset;

static void _MDWTempRunoff (int itemID) {
// Input
	float surfaceRO   = MFVarGetFloat (_MDInCore_SurfRunoffID,  itemID, 0.0);
	float baseFlow    = MFVarGetFloat (_MDInCore_BaseFlowID,    itemID, 0.0);
	float surfRunoffT = MFVarGetFloat (_MDInWTemp_SurfRunoffID, itemID, 0.0);
	float baseFlowT   = MFVarGetFloat (_MDInWTemp_GrdWaterID,   itemID, 0.0);
// Output
	float runoffTemp;

	surfaceRO = MDMaximum (surfaceRO, 0.0);
	baseFlow  = MDMaximum (baseFlow,  0.0);
	runoffTemp =  surfaceRO + baseFlow > 0.0 ? (surfRunoffT * surfaceRO + baseFlowT * baseFlow) / (surfaceRO + baseFlow) : surfRunoffT;
	MFVarSetFloat(_MDOutWTemp_RunoffID,itemID,runoffTemp);
}

enum { MDhelp, MDinput, MDcalculate, MDsurface };

int MDWTemp_RunoffDef () {
    int optID = MFcalculate;
    const char *optStr;
	const char *options [] = { MFhelpStr, MFinputStr, MFcalculateStr, "surface", (char *) NULL };

	if (_MDOutWTemp_RunoffID != MFUnset) return (_MDOutWTemp_RunoffID);

	MFDefEntering ("Runoff temperature");
    if ((optStr = MFOptionGet (MDVarWTemp_Runoff)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
	switch (optID) {
		default:
		case MDhelp:  MFOptionMessage (MDVarWTemp_Runoff, optStr, options); return (CMfailed);
		case MDinput: _MDOutWTemp_RunoffID = MFVarGetID (MDVarWTemp_Runoff, "degC", MFInput, MFState, MFBoundary); break;
		case MDcalculate:
			if (((_MDInCore_SurfRunoffID  = MDCore_RainSurfRunoffDef ()) == CMfailed) ||
			    ((_MDInCore_BaseFlowID    = MDCore_BaseFlowDef ())       == CMfailed) ||
        	    ((_MDInWTemp_GrdWaterID   = MDWTemp_GrdWaterDef ())      == CMfailed) ||
        	    ((_MDInWTemp_SurfRunoffID = MDWTemp_SurfRunoffDef ())    == CMfailed) ||
        	    ((_MDOutWTemp_RunoffID    = MFVarGetID (MDVarWTemp_Runoff, "degC", MFOutput, MFState, MFBoundary)) == CMfailed) ||
        	    (MFModelAddFunction (_MDWTempRunoff) == CMfailed)) return (CMfailed);
			break;
		case MDsurface: _MDOutWTemp_RunoffID = MDWTemp_SurfRunoffDef (); break;
	}
	MFDefLeaving ("Runoff temperature");
	return (_MDOutWTemp_RunoffID);
}
