/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDAux_AvgNSteps.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Output
static int _MDOutAux_StepCounterID = MFUnset;

static void _MDAux_StepCounter (int itemID) {
	int nStep = MFVarGetInt (_MDOutAux_StepCounterID, itemID, 0);
	MFVarSetInt (_MDOutAux_StepCounterID, itemID, nStep + 1);
}

int MDAux_StepCounterDef () {

	if (_MDOutAux_StepCounterID != MFUnset) return (_MDOutAux_StepCounterID);
	MFDefEntering ("Step Count");
	if (((_MDOutAux_StepCounterID = MFVarGetID (MDVarAux_StepCounter, MFNoUnit, MFInt, MFState, MFInitial)) == CMfailed) ||
        (MFModelAddFunction(_MDAux_StepCounter) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Average NSteps");
	return (_MDOutAux_StepCounterID);
}
