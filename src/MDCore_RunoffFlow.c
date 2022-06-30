/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDCore_RunoffVolume.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInCore_RunoffID      = MFUnset;
// Output
static int _MDOutCore_RunoffFlowID = MFUnset;

static void _MDRunoffFlow (int itemID) {
// Input
	float runoff = MFVarGetFloat (_MDInCore_RunoffID, itemID, 0.0) * MFModelGetArea (itemID) / (MFModelGet_dt () * 1000.0);

	MFVarSetFloat (_MDOutCore_RunoffFlowID, itemID, runoff);
}
 
int MDCore_RunoffFlowDef () {
	int optID = MFcalculate;
	const char *optStr;

	if (_MDOutCore_RunoffFlowID != MFUnset) return (_MDOutCore_RunoffFlowID);

	MFDefEntering ("Runoff Flow");
	if ((optStr = MFOptionGet (MDVarCore_RunoffFlow)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:
		case MFhelp:   MFOptionMessage (MDVarCore_RunoffFlow, optStr, MFsourceOptions); return (CMfailed);
		case MFinput: _MDOutCore_RunoffFlowID = MFVarGetID (MDVarCore_RunoffFlow, "m3/s", MFInput, MFState, MFBoundary); break;
		case MFcalculate:
			if (((_MDInCore_RunoffID      = MDCore_RunoffDef()) == CMfailed) ||
                ((_MDOutCore_RunoffFlowID = MFVarGetID (MDVarCore_RunoffFlow, "m3/s", MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDRunoffFlow) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving  ("Runoff Flow");
	return (_MDOutCore_RunoffFlowID);
}
