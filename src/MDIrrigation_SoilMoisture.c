/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDIrrSoilMoisture.c

dominik.wisser@unh.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDOutIrrSoilMoistureID = MFUnset;

int MDIrrigation_SoilMoistDef() {
	int optID = MFnone, ret;
	const char *optStr;

	if (_MDOutIrrSoilMoistureID != MFUnset) return (_MDOutIrrSoilMoistureID);

	if ((optStr = MFOptionGet (MDOptConfig_Irrigation)) != (char *) NULL) optID = CMoptLookup (MFcalcOptions, optStr, true);

	switch (optID) {
		default:      return CMfailed;
		case MFhelp:
		case MFnone:
		case MFinput: break;
		case MFcalculate:
			if ((ret = MDIrrigation_GrossDemandDef ()) == CMfailed) return (CMfailed);
			_MDOutIrrSoilMoistureID = ret != MFUnset ? MFVarGetID (MDVarIrrigation_SoilMoisture, "mm", MFInput, MFFlux, MFBoundary) : MFUnset;
			break;
	}
    return (_MDOutIrrSoilMoistureID);
}
