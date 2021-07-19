/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDIrrSoilMoisture.c

dominik.wisser@unh.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDOutIrrSoilMoistChgID = MFUnset;

enum { MDinput, MDcalculate, MDnone, MDhelp };

int MDIrrigation_SoilMoistChgDef() {
	int optID = MDnone, ret;
	const char *optStr, *optName = MDOptConfig_Irrigation;
	const char *options [] = { MFinputStr, MFcalculateStr, MFnoneStr, MFhelpStr, (char *) NULL };

	if (_MDOutIrrSoilMoistChgID != MFUnset) return (_MDOutIrrSoilMoistChgID);
	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
		
	switch (optID) {
		case MDhelp:
		case MDnone: break;
		case MDinput: _MDOutIrrSoilMoistChgID = MFVarGetID (MDVarIrrigation_SoilMoistChange, "mm", MFInput, MFFlux, MFBoundary); break;
		case MDcalculate: 
			if ((ret = MDIrrigation_GrossDemandDef()) == CMfailed) return CMfailed;
			_MDOutIrrSoilMoistChgID = ret != MFUnset ? MFVarGetID (MDVarIrrigation_SoilMoistChange, "mm", MFInput, MFFlux, MFBoundary) : MFUnset;
			break;
		default: MFOptionMessage (optName, optStr, options); return CMfailed;
	}
    return (_MDOutIrrSoilMoistChgID);
}
