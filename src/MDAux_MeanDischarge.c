/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDDischMean.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInAux_AccumRunoffID = MFUnset;
static int _MDAux_InAvgNStepsID   = MFUnset;

static int _MDOutAux_MeanDischargeID     = MFUnset;

static void _MDAux_MeanDischarge (int itemID) {
	int   nSteps;
	float accumDisch;
	float dischMean;

	accumDisch = MFVarGetFloat (_MDInAux_AccumRunoffID,  itemID, 0.0);
	nSteps     = MFVarGetInt   (_MDAux_InAvgNStepsID,       itemID,   0);
	dischMean  = MFVarGetFloat (_MDOutAux_MeanDischargeID,      itemID, 0.0);
	dischMean  = (float) (((double) dischMean * (double) nSteps + accumDisch) / ((double) (nSteps + 1)));
	MFVarSetFloat (_MDOutAux_MeanDischargeID, itemID, dischMean);
}

int MDAux_MeanDiscargehDef () {
	int  optID = MFinput;
	const char *optStr;

	if (_MDOutAux_MeanDischargeID != MFUnset) return (_MDOutAux_MeanDischargeID);

	MFDefEntering ("Discharge Mean");
	if ((optStr = MFOptionGet (MDVarAux_DischMean)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:      MFOptionMessage (MDVarAux_DischMean, optStr, MFsourceOptions); return (CMfailed);
		case MFhelp:  MFOptionMessage (MDVarAux_DischMean, optStr, MFsourceOptions);
		case MFinput: _MDOutAux_MeanDischargeID  = MFVarGetID (MDVarAux_DischMean, "m3/s", MFInput, MFState, MFBoundary); break;
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
