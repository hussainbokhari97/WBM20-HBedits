/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDRouting_DischargeRelease.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInRouting_DischargeUptakeID = MFUnset;
static int _MDInRouting_DischReleasedID   = MFUnset;

// Output
static int _MDOutRouting_DischargeID = MFUnset;

static void _MDRouting_DischRelease (int itemID) {
	float discharge;

	if ((_MDInRouting_DischReleasedID != MFUnset) && (!MFVarTestMissingVal (_MDInRouting_DischReleasedID, itemID)))
		 discharge = MFVarGetFloat (_MDInRouting_DischReleasedID,   itemID, 0.0);
	else discharge = MFVarGetFloat (_MDInRouting_DischargeUptakeID, itemID, 0.0);

	MFVarSetFloat (_MDOutRouting_DischargeID, itemID, discharge);
}

int MDRouting_DischargeReleaseDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutRouting_DischargeID != MFUnset) return (_MDOutRouting_DischargeID);

	MFDefEntering ("Discharge - Reservoir Release");
	if ((_MDInRouting_DischargeUptakeID = MDRouting_DischargeUptakeDef ()) == CMfailed) return (CMfailed);
	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
	switch (optID) {
		default:     MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFsourceOptions); return (CMfailed);
		case MFhelp: MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFsourceOptions);
		case MFoff:  break;
		case MFon:   if ((_MDInRouting_DischReleasedID = MDReservoir_ReleaseDef()) == CMfailed) return (CMfailed);
	}
	if (((_MDOutRouting_DischargeID     = MFVarGetID ("__DischLevel1", "m3/s",  MFOutput,  MFState, MFBoundary)) == CMfailed) ||
	    (MFModelAddFunction(_MDRouting_DischRelease) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Discharge - Reservoir Release");
	return (_MDOutRouting_DischargeID);
}
