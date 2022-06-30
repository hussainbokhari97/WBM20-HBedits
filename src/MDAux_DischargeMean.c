/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDAux_MeanDischarge.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInAux_AccumRunoffID    = MFUnset;
static int _MDAux_InAvgNStepsID      = MFUnset;

static int _MDOutAux_MeanDischargeID = MFUnset;

static void _MDAux_MeanDischarge (int itemID) {
	int   nSteps     = MFVarGetInt   (_MDAux_InAvgNStepsID,      itemID,   0);
	float accumDisch = MFVarGetFloat (_MDInAux_AccumRunoffID,    itemID, 0.0);
	float dischMean  = MFVarGetFloat (_MDOutAux_MeanDischargeID, itemID, 0.0);

	dischMean  = (float) (((double) dischMean * (double) nSteps + accumDisch) / ((double) (nSteps + 1)));
	MFVarSetFloat (_MDOutAux_MeanDischargeID, itemID, dischMean);
}

int MDAux_MeanDischargeDef () {
	int  optID = MFinput;
	const char *optStr;

	if (_MDOutAux_MeanDischargeID != MFUnset) return (_MDOutAux_MeanDischargeID);

	MFDefEntering ("Discharge Mean");
	if ((optStr = MFOptionGet (MDVarAux_DischMean)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:
		case MFhelp:  MFOptionMessage (MDVarAux_DischMean, optStr, MFsourceOptions); return (CMfailed);
		case MFinput: _MDOutAux_MeanDischargeID = MFVarGetID (MDVarAux_DischMean, "m3/s", MFInput, MFState, MFInitial); break;
		case MFcalculate:
			if (((_MDInAux_AccumRunoffID    = MDAux_AccumRunoffDef()) == CMfailed) ||
			    ((_MDAux_InAvgNStepsID      = MDAux_AvgNStepsDef())   == CMfailed) ||
                ((_MDOutAux_MeanDischargeID = MFVarGetID (MDVarAux_DischMean, "m3/s", MFOutput, MFState, MFInitial)) == CMfailed) ||
                (MFModelAddFunction(_MDAux_MeanDischarge) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Discharge Mean");
	return (_MDOutAux_MeanDischargeID);
}