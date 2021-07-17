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

enum { MDcalculate, MDnone };

int MDIrrigation_UptakeRiverDef() {
	int optID = MDcalculate;
	const char *optStr, *optName = "IrrUptakeRiver";
	const char *options [] = { MDCalculateStr, MDNoneStr, (char *) NULL };

	if ((optID == MDnone) || (_MDOutIrrigation_UptakeRiverID != MFUnset)) return (_MDOutIrrigation_UptakeRiverID);

	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
	if (optID == MDcalculate)
		return (MFVarGetID (MDVarIrrigation_UptakeRiver, "mm", MFOutput, MFFlux, MFBoundary));
	else MFOptionMessage (optName, optStr, options);
	return (CMfailed);
}
