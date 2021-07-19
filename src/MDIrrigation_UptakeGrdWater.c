/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDIrrUptakeGrdWater.c

dominik.wisser@unh.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDOutCommon_IrrUptakeGrdWaterID = MFUnset;

enum { MDcalculate, MDnone, MDhelp };

int MDIrrigation_UptakeGrdWaterDef() {
	int optID = MDcalculate;
	const char *optStr, *optName = "IrrUptakeGrdWater";
	const char *options [] = { MFcalculateStr, MFnoneStr, MFhelpStr, (char *) NULL };

	if (_MDOutCommon_IrrUptakeGrdWaterID != MFUnset) return (_MDOutCommon_IrrUptakeGrdWaterID);

	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
		
	switch (optID) {
		case MDhelp: MFOptionMessage (optName, optStr, options);
		case MDcalculate:
			_MDOutCommon_IrrUptakeGrdWaterID = MFVarGetID (MDVarIrrigation_UptakeGrdWater, "mm", MFOutput, MFFlux, MFBoundary);
			break;
		case MDnone: break;
		default: MFOptionMessage (optName, optStr, options); return CMfailed;
	}
	return (_MDOutCommon_IrrUptakeGrdWaterID);
}
