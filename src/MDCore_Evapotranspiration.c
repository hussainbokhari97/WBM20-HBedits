/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDCore_Evapotranspiration.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInRainEvapotranspID = MFUnset;
static int _MDInIrrEvapotranspID  = MFUnset;
// Output
static int _MDOutEvapotranspID    = MFUnset;

static void _MDEvapotransp (int itemID) {	
// Input
	float et = MFVarGetFloat (_MDInRainEvapotranspID, itemID, 0.0); // Evapotranspiration [mm/dt]
	
	if (_MDInIrrEvapotranspID != MFUnset) et += MFVarGetFloat (_MDInIrrEvapotranspID, itemID, 0.0);
	MFVarSetFloat (_MDOutEvapotranspID,  itemID, et);
}

int MDCore_EvapotranspirationDef () {
	int optID = MFcalculate, ret;
	const char *optStr;

	if (_MDOutEvapotranspID != MFUnset) return (_MDOutEvapotranspID);

	MFDefEntering ("Evapotranspiration");
	if ((optStr = MFOptionGet (MDVarCore_Evapotranspiration)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:
		case MFhelp: MFOptionMessage (MDVarCore_Evapotranspiration, optStr, MFsourceOptions); return (CMfailed);
		case MFinput:
			if ((_MDOutEvapotranspID     = MFVarGetID (MDVarCore_Evapotranspiration, "mm", MFInput, MFFlux, MFBoundary))  == CMfailed) return (CMfailed);
			break;
		case MFcalculate:
			if (((_MDInRainEvapotranspID = MDCore_RainEvapotranspirationDef ())   == CMfailed) ||
				((_MDInIrrEvapotranspID  = MDIrrigation_EvapotranspirationDef ()) == CMfailed) ||
		        ((_MDOutEvapotranspID    = MFVarGetID (MDVarCore_Evapotranspiration, "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
			    (MFModelAddFunction (_MDEvapotransp) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Evapotranspiration");
	return (_MDOutEvapotranspID);
}
