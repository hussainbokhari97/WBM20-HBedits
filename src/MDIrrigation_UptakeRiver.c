/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDIrrUptakeRiver.c

dominik.wisser@unh.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDOutIrrigation_UptakeRiverID = MFUnset;

int MDIrrigation_UptakeRiverDef () {
	const char *optStr;
	int optID = MFoff;

	if (_MDOutIrrigation_UptakeRiverID != MFUnset) return (_MDOutIrrigation_UptakeRiverID);
	if ((optStr = MFOptionGet (MDVarIrrigation_UptakeRiver)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
	switch (optID) {
		default:
		case MFhelp: MFOptionMessage (MDVarIrrigation_UptakeRiver, optStr, MFswitchOptions); return CMfailed;
		case MFoff: break;
		case MFon: _MDOutIrrigation_UptakeRiverID = MFVarGetID (MDVarIrrigation_UptakeRiver, "mm", MFOutput, MFFlux, MFBoundary); break;
	}
	return (_MDOutIrrigation_UptakeRiverID);
}
