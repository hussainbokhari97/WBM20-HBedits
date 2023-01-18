/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDAux_DischargeMin.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInAux_AccumRunoffID = MFUnset;
static int _MDInAux_StepCounterID = MFUnset;

static int _MDOutAux_DischargeMinID = MFUnset;

static void _MDAux_DischargeMin (int itemID) {
	int   tStep      = MFVarGetInt   (_MDInAux_StepCounterID,       itemID, 0);
	float discharge  = MFVarGetFloat (_MDOutAux_DischargeMinID, itemID, 0.0);
	float accumDisch = tStep > 0 ? MFVarGetFloat (_MDInAux_AccumRunoffID, itemID, 0.0) : discharge;

	discharge  = discharge < accumDisch ? discharge : accumDisch;
	MFVarSetFloat (_MDOutAux_DischargeMinID, itemID, discharge);
}

int MDAux_DischargeMinDef () {
	int  optID = MFinput;
	const char *optStr;

	if (_MDOutAux_DischargeMinID != MFUnset) return (_MDOutAux_DischargeMinID);

	MFDefEntering ("Discharge Minimum");
	if ((optStr = MFOptionGet (MDVarAux_DischargeMean)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:
		case MFhelp:  MFOptionMessage (MDVarAux_DischargeMean, optStr, MFsourceOptions); return (CMfailed);
		case MFinput: _MDOutAux_DischargeMinID = MFVarGetID (MDVarAux_DischargeMean, "m3/s", MFInput, MFState, MFInitial); break;
		case MFcalculate:
			if (((_MDInAux_AccumRunoffID   = MDAux_AccumRunoffDef()) == CMfailed) ||
				((_MDInAux_StepCounterID   = MDAux_StepCounterDef()) == CMfailed) ||
                ((_MDOutAux_DischargeMinID = MFVarGetID (MDVarAux_DischargeMin,  "m3/s", MFOutput, MFState, MFInitial)) == CMfailed) ||
                (MFModelAddFunction(_MDAux_DischargeMin) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Discharge Minimum");
	return (_MDOutAux_DischargeMinID);
}
