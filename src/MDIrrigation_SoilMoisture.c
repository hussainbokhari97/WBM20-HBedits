/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDIrrigation_SoilMoisture.c

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
		default:
		case MFhelp: MFOptionMessage (MDOptConfig_Irrigation, optStr, MFcalcOptions); return (CMfailed);
		case MFnone:
		case MFinput:
			_MDOutIrrSoilMoistureID = MFVarGetID (MDVarIrrigation_SoilMoisture, "mm", MFInput, MFFlux, MFBoundary);
			break;
		case MFcalculate:
			if ((ret = MDIrrigation_GrossDemandDef ()) == CMfailed) return (CMfailed);
			_MDOutIrrSoilMoistureID = ret != MFUnset ? MFVarGetID (MDVarIrrigation_SoilMoisture, "mm", MFInput, MFFlux, MFBoundary) : MFUnset;
			break;
	}
    return (_MDOutIrrSoilMoistureID);
}
