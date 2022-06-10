/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDCore_RunoffVolume.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInCore_RunoffID        = MFUnset;
// Output
static int _MDOutRunoffVolumeID = MFUnset;

static void _MDRunoffVolume (int itemID) {
// Input
	float runoff;

	runoff = MFVarGetFloat (_MDInCore_RunoffID, itemID, 0.0) * MFModelGetArea (itemID) / (MFModelGet_dt () * 1000.0);
//	if((itemID == 25014) && (runoff * 86400 < -0.000009)) printf("############ runoff = %f\n", runoff * 86400);  //runoff = 0.0;							//RJS 071511
	MFVarSetFloat (_MDOutRunoffVolumeID, itemID, runoff);
}
 
int MDCore_RunoffVolumeDef () {
	int optID = MFinput;
	const char *optStr;

	if (_MDOutRunoffVolumeID != MFUnset) return (_MDOutRunoffVolumeID);

	MFDefEntering ("Runoff Volume");
	if ((optStr = MFOptionGet (MDVarCore_RunoffVolume)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:       MFOptionMessage (MDVarCore_RunoffVolume, optStr, MFsourceOptions); return (CMfailed);
		case MFhelp:   MFOptionMessage (MDVarCore_RunoffVolume, optStr, MFsourceOptions); 
		case MFinput: _MDOutRunoffVolumeID = MFVarGetID (MDVarCore_RunoffVolume, "m3/s", MFInput, MFState, MFBoundary); break;
		case MFcalculate:
			if (((_MDInCore_RunoffID   = MDCore_RunoffDef()) == CMfailed) ||
                ((_MDOutRunoffVolumeID = MFVarGetID (MDVarCore_RunoffVolume, "m3/s", MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDRunoffVolume) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving  ("Runoff Volume");
	return (_MDOutRunoffVolumeID);
}
