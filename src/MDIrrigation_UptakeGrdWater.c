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

enum { MDhelp, MDoff, MDon };

int MDIrrigation_UptakeGrdWaterDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutCommon_IrrUptakeGrdWaterID != MFUnset) return (_MDOutCommon_IrrUptakeGrdWaterID);
	if ((optStr = MFOptionGet ("IrrUptakeGrdWater")) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
	switch (optID) {
		default:     MFOptionMessage ("IrrUptakeGrdWater", optStr, MFswitchOptions); return CMfailed;
		case MFhelp: MFOptionMessage ("IrrUptakeGrdWater", optStr, MFswitchOptions);
		case MFoff: break;
		case MFon: _MDOutCommon_IrrUptakeGrdWaterID = MFVarGetID (MDVarIrrigation_UptakeGrdWater, "mm", MFOutput, MFFlux, MFBoundary); break;
	}
	return (_MDOutCommon_IrrUptakeGrdWaterID);
}
