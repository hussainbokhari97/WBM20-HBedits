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
static int _MDOutCore_RunoffVolumeID = MFUnset;

static void _MDRunoffVolume (int itemID) {
// Input
	float runoff;

	runoff = MFVarGetFloat (_MDInCore_RunoffID, itemID, 0.0) * MFModelGetArea (itemID) / (MFModelGet_dt () * 1000.0);
	MFVarSetFloat (_MDOutCore_RunoffVolumeID, itemID, runoff);
}
 
int MDCore_RunoffVolumeDef () {
	int optID = MFinput;
	const char *optStr;

	if (_MDOutCore_RunoffVolumeID != MFUnset) return (_MDOutCore_RunoffVolumeID);

	MFDefEntering ("Runoff Volume");
	if ((optStr = MFOptionGet (MDVarCore_RunoffVolume)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:       MFOptionMessage (MDVarCore_RunoffVolume, optStr, MFsourceOptions); return (CMfailed);
		case MFhelp:   MFOptionMessage (MDVarCore_RunoffVolume, optStr, MFsourceOptions); 
		case MFinput: _MDOutCore_RunoffVolumeID = MFVarGetID (MDVarCore_RunoffVolume, "m3/s", MFInput, MFState, MFBoundary); break;
		case MFcalculate:
			if (((_MDInCore_RunoffID        = MDCore_RunoffDef()) == CMfailed) ||
                ((_MDOutCore_RunoffVolumeID = MFVarGetID (MDVarCore_RunoffVolume, "m3/s", MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDRunoffVolume) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving  ("Runoff Volume");
	return (_MDOutCore_RunoffVolumeID);
}
