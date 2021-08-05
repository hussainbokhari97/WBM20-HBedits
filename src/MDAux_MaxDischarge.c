/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDAux_MaxDischarge.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInAux_AccumRunoffID       = MFUnset;

static int _MDOutAux_MaximumDischargeID = MFUnset;

static void _MDAux_MaximumDischarge (int itemID) {
	float accumDisch;
	float discharge;

	accumDisch = MFVarGetFloat (_MDInAux_AccumRunoffID,       itemID, 0.0);
	discharge  = MFVarGetFloat (_MDOutAux_MaximumDischargeID, itemID, 0.0);
	discharge  = discharge > accumDisch ? discharge : accumDisch;
	MFVarSetFloat (_MDOutAux_MaximumDischargeID, itemID, discharge);
}

int MDAux_MaximumDiscargehDef () {
	int  optID = MFinput;
	const char *optStr;

	if (_MDOutAux_MaximumDischargeID != MFUnset) return (_MDOutAux_MaximumDischargeID);

	MFDefEntering ("Discharge Mean");
	if ((optStr = MFOptionGet (MDVarAux_DischMean)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:      MFOptionMessage (MDVarAux_DischMean, optStr, MFsourceOptions); return (CMfailed);
		case MFhelp:  MFOptionMessage (MDVarAux_DischMean, optStr, MFsourceOptions);
		case MFinput: _MDOutAux_MaximumDischargeID = MFVarGetID (MDVarAux_DischMean, "m3/s", MFInput, MFState, MFBoundary); break;
		case MFcalculate:
			if (((_MDInAux_AccumRunoffID        = MDAux_AccumRunoffDef()) == CMfailed) ||
                ((_MDOutAux_MaximumDischargeID  = MFVarGetID (MDVarAux_DischMax,  "m3/s", MFOutput, MFState, MFInitial)) == CMfailed) ||
                (MFModelAddFunction(_MDAux_MaximumDischarge) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Discharge Mean");
	return (_MDOutAux_MaximumDischargeID);
}
