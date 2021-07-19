/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDIrrUptakeRiver.c

dominik.wisser@unh.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDOutIrrigation_UptakeRiverID = MFUnset;

enum { MDcalculate, MDnone, MDhelp };

int MDIrrigation_UptakeRiverDef() {
	int optID = MDcalculate;
	const char *optStr, *optName = "IrrUptakeRiver";
	const char *options [] = { MFcalculateStr, MFnoneStr, MFhelpStr, (char *) NULL };

	if (_MDOutIrrigation_UptakeRiverID != MFUnset) return (_MDOutIrrigation_UptakeRiverID);

	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
	switch (optID) {
		case MDhelp:      MFOptionMessage (optName, optStr, options);
		case MDcalculate: _MDOutIrrigation_UptakeRiverID = MFVarGetID (MDVarIrrigation_UptakeRiver, "mm", MFOutput, MFFlux, MFBoundary); break;
		case MDnone: break;
		default: MFOptionMessage (optName, optStr, options); return CMfailed;
	}
	return (_MDOutIrrigation_UptakeRiverID);
}
