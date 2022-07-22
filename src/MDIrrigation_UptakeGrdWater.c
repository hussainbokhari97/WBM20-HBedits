/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDIrrUptakeGrdWater.c

dominik.wisser@unh.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDOutCommon_IrrUptakeGrdWaterID = MFUnset;

int MDIrrigation_UptakeGrdWaterDef () {
	int optID = MFon;
	const char *optStr;

	if (_MDOutCommon_IrrUptakeGrdWaterID != MFUnset) return (_MDOutCommon_IrrUptakeGrdWaterID);
	if ((optStr = MFOptionGet (MDVarIrrigation_UptakeGrdWater)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
	switch (optID) {
		default:
		case MFhelp: MFOptionMessage (MDVarIrrigation_UptakeGrdWater, optStr, MFswitchOptions); return CMfailed;
		case MFoff: break;
		case MFon: _MDOutCommon_IrrUptakeGrdWaterID = MFVarGetID (MDVarIrrigation_UptakeGrdWater, "mm", MFOutput, MFFlux, MFBoundary); break;
	}
	return (_MDOutCommon_IrrUptakeGrdWaterID);
}
