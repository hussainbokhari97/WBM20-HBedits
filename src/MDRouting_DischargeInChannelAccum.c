/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDRouting_DichargeInChannelAccumulate.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInCore_RunoffVolumeID      = MFUnset;
static int _MDInRouting_DischargeID     = MFUnset;
// Output
static int _MDOutRouting_DischLevel3ID  = MFUnset;
static int _MDOutRouting_RiverStorChgID = MFUnset;
static int _MDOutRouting_RiverStorageID = MFUnset;

static void _MDDischLevel3Accumulate (int itemID) {
// Input
	float runoff;     // Local runoff volume [m3/s]
	float discharge;  // Discharge from upstream [m3/s]

	runoff    = MFVarGetFloat(_MDInCore_RunoffVolumeID, itemID, 0.0);
	discharge = MFVarGetFloat(_MDInRouting_DischargeID,    itemID, 0.0);

	MFVarSetFloat (_MDOutRouting_DischLevel3ID, itemID, discharge + runoff);
	MFVarSetFloat (_MDOutRouting_RiverStorChgID, itemID, 0.0);
	MFVarSetFloat (_MDOutRouting_RiverStorageID, itemID, 0.0);
}

int MDRouting_DischargeInChannelAccumulateDef () {

	if (_MDOutRouting_DischLevel3ID != MFUnset) return (_MDOutRouting_DischLevel3ID);

	MFDefEntering ("Discharge Routing - Accumulate");
	if (((_MDInCore_RunoffVolumeID  = MDCore_RunoffVolumeDef()) == CMfailed) ||
        ((_MDInRouting_DischargeID     = MFVarGetID (MDVarRouting_Discharge,       "m3/s", MFInput,  MFState, MFInitial))  == CMfailed) ||
        ((_MDOutRouting_DischLevel3ID  = MFVarGetID ("__DischLevel3",              "m3/s", MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutRouting_RiverStorChgID = MFVarGetID (MDVarRouting_RiverStorageChg, "m3",   MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutRouting_RiverStorageID     = MFVarGetID (MDVarRouting_RiverStorage,    "m3",   MFOutput, MFState, MFInitial))  == CMfailed) ||
        (MFModelAddFunction (_MDDischLevel3Accumulate) == CMfailed)) return CMfailed;
	MFDefLeaving ("Discharge Routing - Accumulate");
	return (_MDOutRouting_DischLevel3ID);
}
