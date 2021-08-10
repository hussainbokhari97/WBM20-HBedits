/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDAux_MaxDischarge.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInAux_AccumRunoffID = MFUnset;
static int _MDAux_InAvgNStepsID   = MFUnset;

static int _MDOutAux_MinimumDischargeID = MFUnset;

static void _MDAux_MinimumDischarge (int itemID) {
	int   nSteps;
	float accumDisch;
	float discharge;

	nSteps     = MFVarGetInt   (_MDAux_InAvgNStepsID,         itemID, 0);
	discharge  = MFVarGetFloat (_MDOutAux_MinimumDischargeID, itemID, 0.0);
	accumDisch = nSteps > 0 ? MFVarGetFloat (_MDInAux_AccumRunoffID, itemID, 0.0) : discharge;
	discharge  = discharge < accumDisch ? discharge : accumDisch;
	MFVarSetFloat (_MDOutAux_MinimumDischargeID, itemID, discharge);
}

int MDAux_MinimumDischargeDef () {
	int  optID = MFinput;
	const char *optStr;

	if (_MDOutAux_MinimumDischargeID != MFUnset) return (_MDOutAux_MinimumDischargeID);

	MFDefEntering ("Discharge Minimum");
	if ((optStr = MFOptionGet (MDVarAux_DischMean)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:      MFOptionMessage (MDVarAux_DischMean, optStr, MFsourceOptions); return (CMfailed);
		case MFhelp:  MFOptionMessage (MDVarAux_DischMean, optStr, MFsourceOptions);
		case MFinput: _MDOutAux_MinimumDischargeID = MFVarGetID (MDVarAux_DischMean, "m3/s", MFInput, MFState, MFInitial); break;
		case MFcalculate:
			if (((_MDInAux_AccumRunoffID        = MDAux_AccumRunoffDef()) == CMfailed) ||
				((_MDAux_InAvgNStepsID          = MDAux_AvgNStepsDef())   == CMfailed) ||
                ((_MDOutAux_MinimumDischargeID  = MFVarGetID (MDVarAux_DischMax,  "m3/s", MFOutput, MFState, MFInitial)) == CMfailed) ||
                (MFModelAddFunction(_MDAux_MinimumDischarge) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Discharge Minimum");
	return (_MDOutAux_MinimumDischargeID);
}
