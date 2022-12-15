/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDAux_MeanDischarge.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInAux_StepCounterID    = MFUnset;
static int _MDInAux_AccumRunoffID    = MFUnset;

static int _MDOutAux_DischargeMeanID = MFUnset;

static void _MDAux_DischargeMean (int itemID) {
	int   tStep      = MFVarGetInt   (_MDInAux_StepCounterID,    itemID,   0);
	float accumDisch = MFVarGetFloat (_MDInAux_AccumRunoffID,    itemID, 0.0);
	float dischMean  = MFVarGetFloat (_MDOutAux_DischargeMeanID, itemID, 0.0);

	dischMean  = (float) (((double) dischMean * (double) tStep + accumDisch) / ((double) (tStep + 1)));
	MFVarSetFloat (_MDOutAux_DischargeMeanID, itemID, dischMean);
}

int MDAux_DischargeMeanDef () {
	int  optID = MFcalculate;
	const char *optStr;

	if (_MDOutAux_DischargeMeanID != MFUnset) return (_MDOutAux_DischargeMeanID);

	MFDefEntering ("Discharge Mean");
	if ((optStr = MFOptionGet (MDVarAux_DischargeMean)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:
		case MFhelp:  MFOptionMessage (MDVarAux_DischargeMean, optStr, MFsourceOptions); return (CMfailed);
		case MFinput: _MDOutAux_DischargeMeanID = MFVarGetID (MDVarAux_DischargeMean, "m3/s", MFInput, MFState, MFBoundary); break;
		case MFcalculate:
			if (((_MDInAux_StepCounterID    = MDAux_StepCounterDef()) == CMfailed) ||
			    ((_MDInAux_AccumRunoffID    = MDAux_AccumRunoffDef()) == CMfailed) ||
                ((_MDOutAux_DischargeMeanID = MFVarGetID (MDVarAux_DischargeMean, "m3/s", MFOutput, MFState, MFInitial)) == CMfailed) ||
                (MFModelAddFunction(_MDAux_DischargeMean) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Discharge Mean");
	return (_MDOutAux_DischargeMeanID);
}