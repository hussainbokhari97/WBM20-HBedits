/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDIrrRefET.c

dominink.wisser@unh.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDOutIrrRefEvapotransID = MFUnset;

enum { MDhelp, MDinput, MDhamon, MDfao };

int MDIrrigation_ReferenceETDef () {
	int optID = MDinput;
	const char *optStr;
	const char *options [] = { MFhelpStr, MFinputStr, "Hamon", "FAO", (char *) NULL };

	if (_MDOutIrrRefEvapotransID != MFUnset) return (_MDOutIrrRefEvapotransID);

	MFDefEntering ("Irrigation Reference Evapotranspiration");
	if ((optStr = MFOptionGet (MDOptIrrigation_ReferenceET)) != (char *) NULL) optID = CMoptLookup (options,optStr,true);
	if (MDCore_SnowPackChgDef() == CMfailed) return (CMfailed);

	switch (optID) {
		default:
		case MDhelp:  MFOptionMessage (MDOptIrrigation_ReferenceET, optStr, options); return (CMfailed);
		case MDinput: _MDOutIrrRefEvapotransID = MFVarGetID (MDVarIrrigation_RefEvapotrans, "mm", MFInput, MFFlux, false); break;
		case MDhamon: _MDOutIrrRefEvapotransID = MDIrrigation_Reference_ETHamonDef(); break;
		case MDfao:   _MDOutIrrRefEvapotransID = MDIrrigation_ReferenceETFAODef(); break;
	}
	MFDefLeaving ("Irrigation Reference Evapotranspiration");
	return (_MDOutIrrRefEvapotransID);
}
