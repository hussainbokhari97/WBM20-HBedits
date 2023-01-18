/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDAyx_RunoffMean.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInCore_RunoffID     = MFUnset;
static int _MDInAux_StepCounterID = MFUnset;

static int _MDOutAux_RunoffMeanID     = MFUnset;

static void _MDAux_MeanRunoff (int itemID) {
	int   tStep      = MFVarGetInt   (_MDInAux_StepCounterID, itemID, 0);
	float runoff     = MFVarGetFloat (_MDInCore_RunoffID,     itemID, 0.0);
	float runoffMean = MFVarGetFloat (_MDOutAux_RunoffMeanID, itemID, 0.0);

	runoffMean  = (float) (((double) runoffMean * (double) tStep + runoff) / ((double) (tStep + 1)));
	MFVarSetFloat (_MDOutAux_RunoffMeanID, itemID, runoffMean);
}

int MDAux_RunoffMeanDef () {
	int  optID = MFinput;
	const char *optStr;

	if (_MDOutAux_RunoffMeanID != MFUnset) return (_MDOutAux_RunoffMeanID);

	MFDefEntering ("Runoff Mean");
	if ((optStr = MFOptionGet (MDVarCore_RunoffMean)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:
		case MFhelp:  MFOptionMessage (MDVarCore_RunoffMean, optStr, MFsourceOptions); return (CMfailed);
		case MFinput: _MDOutAux_RunoffMeanID  = MFVarGetID (MDVarCore_RunoffMean, "mm/d", MFInput, MFState, MFBoundary); break;
		case MFcalculate:
			if (((_MDInAux_StepCounterID = MDAux_StepCounterDef()) == CMfailed) ||
                ((_MDInCore_RunoffID     = MDCore_RunoffDef())     == CMfailed) ||
                ((_MDOutAux_RunoffMeanID = MFVarGetID (MDVarCore_RunoffMean, "mm/d", MFOutput, MFState, MFInitial)) == CMfailed) ||
                (MFModelAddFunction(_MDAux_MeanRunoff) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Runoff Mean");
	return (_MDOutAux_RunoffMeanID);
}
