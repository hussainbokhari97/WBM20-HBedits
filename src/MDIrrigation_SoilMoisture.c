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

enum { MDinput, MDcalculate, MDnone };

int MDIrrigation_SoilMoistDef() {
	int optID = MDinput, ret;
	const char *optStr, *optName = MDOptConfig_Irrigation;
	const char *options [] = { MDInputStr, MDCalculateStr, MDNoneStr, (char *) NULL };

	if ((optID == MDnone) || (optID == MDinput) || (_MDOutIrrSoilMoistureID != MFUnset)) return (_MDOutIrrSoilMoistureID);

	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
	if ((ret = MDIrrigation_GrossDemandDef()) == CMfailed) return (CMfailed);
	if (ret == MFUnset) return (MFUnset);
	_MDOutIrrSoilMoistureID = MFVarGetID (MDVarIrrigation_SoilMoisture, "mm", MFInput, MFFlux, MFBoundary);
    return (_MDOutIrrSoilMoistureID);
}
