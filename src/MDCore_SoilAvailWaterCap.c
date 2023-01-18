/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDCore_SoilAvailWaterCap.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInSoilFieldCapacityID  = MFUnset;
static int _MDInSoilWiltingPointID   = MFUnset;
static int _MDInSoilRootingDepthID   = MFUnset;

// Output
static int _MDOutSoilAvailWaterCapID = MFUnset;

static void _MDSoilAvailWaterCap (int itemID) {
// Input
	float fieldCapacity = MFVarGetFloat (_MDInSoilFieldCapacityID, itemID, 0.0); // Field capacity [m/m]
	float wiltingPoint  = MFVarGetFloat (_MDInSoilWiltingPointID,  itemID, 0.0); // Wilting point  [m/m]
	float rootingDepth  = MFVarGetFloat (_MDInSoilRootingDepthID,  itemID, 0.0); // Rooting depth  [mm]

	if (fieldCapacity < wiltingPoint) fieldCapacity = wiltingPoint;	
	MFVarSetFloat (_MDOutSoilAvailWaterCapID, itemID, rootingDepth * (fieldCapacity - wiltingPoint));
}

int MDCore_SoilAvailWaterCapDef () {
	int  optID = MFinput;
	const char *optStr;
	
	if (_MDOutSoilAvailWaterCapID != MFUnset) return (_MDOutSoilAvailWaterCapID);

	MFDefEntering ("Soil available water capacity");
	if ((optStr = MFOptionGet (MDVarCore_SoilAvailWaterCap)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
		switch (optID) {
			default:
			case MFhelp:  MFOptionMessage (MDVarCore_SoilAvailWaterCap, optStr, MFsourceOptions); return (CMfailed);
			case MFinput: _MDOutSoilAvailWaterCapID = MFVarGetID (MDVarCore_SoilAvailWaterCap, "mm", MFInput, MFState, MFBoundary); break;
			case MFcalculate:
				if (((_MDInSoilFieldCapacityID  = MFVarGetID (MDVarCore_SoilFieldCapacity,     "mm/m", MFInput,  MFState, MFBoundary)) == CMfailed) ||
                	((_MDInSoilWiltingPointID   = MFVarGetID (MDVarCore_SoilWiltingPoint,      "mm/m", MFInput,  MFState, MFBoundary)) == CMfailed) ||
                	((_MDInSoilRootingDepthID   = MFVarGetID (MDVarCore_SoilRootingDepth,      "mm",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
                	((_MDOutSoilAvailWaterCapID = MFVarGetID (MDVarCore_SoilAvailWaterCap,     "mm",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
                	(MFModelAddFunction (_MDSoilAvailWaterCap) == CMfailed)) return (CMfailed);
				break;
		}
	MFDefLeaving  ("Soil available water capacity");
	return (_MDOutSoilAvailWaterCapID);
}
