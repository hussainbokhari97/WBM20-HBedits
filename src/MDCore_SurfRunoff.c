/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDCore_SurfRunoff.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInRainSurfCore_RunoffID = MFUnset;

// Output
static int _MDOutSurfCore_RunoffID    = MFUnset;

static void _MDSurfRunoff (int itemID) {	
// Input
	float surfRunoff = MFVarGetFloat (_MDInRainSurfCore_RunoffID, itemID, 0.0); // Surface runoff [mm/dt]
	
	MFVarSetFloat (_MDOutSurfCore_RunoffID,  itemID, surfRunoff);
}

int MDCore_SurfRunoffDef () {
	int ret;
	if (_MDOutSurfCore_RunoffID != MFUnset) return (_MDOutSurfCore_RunoffID);

	MFDefEntering ("Surface runoff");	
	if (((_MDInRainSurfCore_RunoffID = MDCore_RainSurfRunoffDef ()) == CMfailed) ||
        ((_MDOutSurfCore_RunoffID    = MFVarGetID (MDVarCore_SurfRunoff, "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDSurfRunoff) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Surface runoff");
	return (_MDOutSurfCore_RunoffID);
}
