/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDCore_Evapotransp.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInRainEvapotranspID = MFUnset;
static int _MDInIrrEvapotranspID  = MFUnset;
static int _MDInSmallResEvapoID   = MFUnset;
// Output
static int _MDOutEvapotranspID    = MFUnset;

static void _MDEvapotransp (int itemID) {	
// Input
	float et=0;    // Evapotranspiration [mm/dt]
	
	et = MFVarGetFloat (_MDInRainEvapotranspID,     itemID, 0.0)
	   + (_MDInIrrEvapotranspID != MFUnset ? MFVarGetFloat (_MDInIrrEvapotranspID, itemID, 0.0) : 0.0)
	   + (_MDInSmallResEvapoID  != MFUnset ? MFVarGetFloat (_MDInSmallResEvapoID,  itemID, 0.0) : 0.0);
//	if (itemID==104)printf ("ETP hier %f irrET %f\n",et,MFVarGetFloat (_MDInIrrEvapotranspID, itemID, 0.0) );
	MFVarSetFloat (_MDOutEvapotranspID,  itemID, et);
}

enum { MDhelp, MDnone, MDinput, MDcalculate, };

int MDCore_EvapotranspirationDef () {
	int optID = MDnone, ret;
	const char *optStr, *optName = MDOptConfig_Irrigation;
	const char *options [] = { MFhelpStr, MFnoneStr, MFinputStr, MFcalculateStr, (char *) NULL };

	if (_MDOutEvapotranspID != MFUnset) return (_MDOutEvapotranspID);

	MFDefEntering ("Evapotranspiration");
	if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
	switch (optID) {
		case MDhelp:
		case MDnone:  
		case MDinput: break;
		case MDcalculate: 
			if ((_MDInIrrEvapotranspID = MFVarGetID (MDVarIrrigation_Evapotranspiration, "mm", MFInput, MFFlux, MFBoundary)) == CMfailed) return (CMfailed);
			if ((ret = MDReservoir_FarmPondReleaseDef ()) != MFUnset) {
				if ((ret == CMfailed) ||
					((_MDInSmallResEvapoID  = MFVarGetID (MDVarReservoir_FarmPondEvaporation, "mm", MFInput, MFFlux, MFBoundary)) == CMfailed))
					return (CMfailed);
			}
			break;
		default: return (CMfailed);
	}
 
	if (((_MDInRainEvapotranspID = MFVarGetID (MDVarCore_RainEvapotranspiration, "mm", MFInput,  MFFlux, MFBoundary)) == CMfailed) ||
		((_MDOutEvapotranspID    = MFVarGetID (MDVarCore_Evapotranspiration,     "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
		(MFModelAddFunction (_MDEvapotransp) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Evapotranspiration");
	return (_MDOutEvapotranspID);
}
