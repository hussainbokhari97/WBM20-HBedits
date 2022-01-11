/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDInfiltration.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <string.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInRainWaterSurplusID       = MFUnset;
// Output
static int _MDOutRainSurfCore_RunoffID   = MFUnset;
static int _MDOutRainInfiltrationID      = MFUnset;
static int _MDInRainInfiltrationID       = MFUnset;

static float _MDInfiltrationFrac = 0.5;  // Water surplus that rechanrges the shallow groundwater pool.

static void _MDRainInfiltrationSimple (int itemID) {

	float surplus;
	float surfRunoff;
	float infiltration;

	surplus = MFVarGetFloat(_MDInRainWaterSurplusID, itemID, 0.0);
	surfRunoff   = surplus * (1.0 - _MDInfiltrationFrac);
	infiltration = surplus *_MDInfiltrationFrac;
	MFVarSetFloat (_MDOutRainSurfCore_RunoffID,  itemID, surfRunoff);
	MFVarSetFloat (_MDOutRainInfiltrationID,     itemID, infiltration);
}

int MDCore_RainInfiltrationDef () {
	const char *optStr;
	float par;

	if (_MDOutRainInfiltrationID != MFUnset) return (_MDOutRainInfiltrationID);
	if ((optStr = MFOptionGet (MDParInfiltrationFrac))  != (char *) NULL) {
		if (strcmp(optStr,MFhelpStr) == 0) CMmsgPrint (CMmsgInfo,"%s = %f", MDParInfiltrationFrac, _MDInfiltrationFrac);
		_MDInfiltrationFrac = sscanf (optStr,"%f",&par) == 1 ? par : _MDInfiltrationFrac;
	}
	MFDefEntering ("Rainfed Infiltration");
	if (((_MDInRainWaterSurplusID = MDCore_RainWaterSurplusDef()) == CMfailed) ||
        ((_MDOutRainSurfCore_RunoffID   = MFVarGetID (MDVarCore_RainSurfRunoff, "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
        ((_MDOutRainInfiltrationID = MFVarGetID (MDVarCore_RainInfiltration, "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDRainInfiltrationSimple) == CMfailed)) return (CMfailed);
	MFDefLeaving  ("Rainfed Infiltration");
	return (_MDOutRainInfiltrationID);
}
