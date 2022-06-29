/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDAyx_MeanDischarge.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInCore_RunoffID = MFUnset;
static int _MDAux_InAvgNStepsID      = MFUnset;

static int _MDOutAux_MeanDischargeID     = MFUnset;

static void _MDAux_MeanRunoff (int itemID) {
	int   nSteps     = MFVarGetInt   (_MDAux_InAvgNStepsID,      itemID, 0);
	float runoff     = MFVarGetFloat (_MDInCore_RunoffID,        itemID, 0.0);
	float runoffMean = MFVarGetFloat (_MDOutAux_MeanDischargeID, itemID, 0.0);

	runoffMean  = (float) (((double) runoffMean * (double) nSteps + runoff) / ((double) (nSteps + 1)));
	MFVarSetFloat (_MDOutAux_MeanDischargeID, itemID, runoffMean);
}

int MDAux_MeanRunoffDef () {
	int  optID = MFinput;
	const char *optStr;

	if (_MDOutAux_MeanDischargeID != MFUnset) return (_MDOutAux_MeanDischargeID);

	MFDefEntering ("Runoff Mean");
	if ((optStr = MFOptionGet (MDVarCore_RunoffMean)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:
		case MFhelp:  MFOptionMessage (MDVarCore_RunoffMean, optStr, MFsourceOptions); return (CMfailed);
		case MFinput: _MDOutAux_MeanDischargeID  = MFVarGetID (MDVarCore_RunoffMean, "mm/d", MFInput, MFState, MFBoundary); break;
		case MFcalculate:
			if (((_MDAux_InAvgNStepsID      = MDAux_AvgNStepsDef()) == CMfailed) ||
                ((_MDInCore_RunoffID        = MDCore_RunoffDef())   == CMfailed) ||
                ((_MDOutAux_MeanDischargeID = MFVarGetID (MDVarCore_RunoffMean, "mm/d", MFOutput, MFState, MFInitial)) == CMfailed) ||
                (MFModelAddFunction(_MDAux_MeanRunoff) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Runoff Mean");
	return (_MDOutAux_MeanDischargeID);
}
