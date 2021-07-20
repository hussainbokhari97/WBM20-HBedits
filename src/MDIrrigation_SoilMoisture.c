/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDIrrSoilMoisture.c

dominik.wisser@unh.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDOutIrrSoilMoistureID = MFUnset;

enum { MDhelp, MDnone, MDinput, MDcalculate };

int MDIrrigation_SoilMoistDef() {
	int optID = MDnone, ret;
	const char *optStr, *optName = MDOptConfig_Irrigation;
	const char *options [] = { MFhelpStr, MFnoneStr, MFinputStr, MFcalculateStr, (char *) NULL };

	if (_MDOutIrrSoilMoistureID != MFUnset) return (_MDOutIrrSoilMoistureID);

	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);

	switch (optID) {
		case MDhelp:
		case MDnone:
		case MDinput: break;
		case MDcalculate:
			if ((ret = MDIrrigation_GrossDemandDef()) == CMfailed) return (CMfailed);
			_MDOutIrrSoilMoistureID = ret != MFUnset ? MFVarGetID (MDVarIrrigation_SoilMoisture, "mm", MFInput, MFFlux, MFBoundary) : MFUnset;
			break;
		default: MFOptionMessage (optName, optStr, options); return CMfailed;
	}
    return (_MDOutIrrSoilMoistureID);
}
