/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDRouting_DichargeInChannelMuskingum.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInCore_RunoffVolumeID   = MFUnset;
static int _MDInRouting_MuskingumC0ID = MFUnset;
static int _MDInRouting_MuskingumC1ID = MFUnset;
static int _MDInRouting_MuskingumC2ID = MFUnset;
static int _MDInRouting_DischargeID   = MFUnset;
// Output
static int _MDOutRouting_Discharge0ID   = MFUnset;
static int _MDOutRouting_Discharge1ID   = MFUnset;
static int _MDOutRouting_DischLevel3ID  = MFUnset;
static int _MDOutRouting_RiverStorChgID = MFUnset;
static int _MDOutRouting_RiverStorageID = MFUnset;

static void _MDDischLevel3Muskingum (int itemID) {
// Input
	float C0;              // Muskingum C0 coefficient (current inflow)
	float C1;              // Muskingum C1 coefficient (previous inflow)
	float C2;              // MUskingum C2 coefficient (previous outflow) 
	float runoff;          // Runoff [mm/dt]
// Output
	float inDischCurrent;  // Upstream discharge at the current time step [m3/s]
	float outDisch;        // Downstream discharge [m3/s]
// Local
	float inDischPrevious; // Upstream discharge at the previous time step [m3/s]
	float storageChg;      // River Storage Change [m3]
	float storage;         // River Storage [m3]
	float dt = MFModelGet_dt ();
	
	C0 = MFVarGetFloat (_MDInRouting_MuskingumC0ID, itemID, 1.0);
	C1 = MFVarGetFloat (_MDInRouting_MuskingumC1ID, itemID, 0.0);
	C2 = MFVarGetFloat (_MDInRouting_MuskingumC2ID, itemID, 0.0);

	runoff          = MFVarGetFloat (_MDInCore_RunoffVolumeID,     itemID, 0.0);
	inDischCurrent  = MFVarGetFloat (_MDInRouting_DischargeID,     itemID, 0.0) + runoff;
 	inDischPrevious = MFVarGetFloat (_MDOutRouting_Discharge0ID,   itemID, 0.0);
	outDisch        = MFVarGetFloat (_MDOutRouting_Discharge1ID,   itemID, 0.0);
	storage         = MFVarGetFloat (_MDOutRouting_RiverStorageID, itemID, 0.0);

	outDisch    = C0 * inDischCurrent + C1 * inDischPrevious + C2 * outDisch;
	outDisch    = outDisch > 0.0 ? outDisch : 0.0; // negative C1 and C2 could cause negative discharge

	storageChg  = (inDischCurrent - outDisch) * dt;
	if (storage + storageChg > 0.0) storage += storageChg;
	else {
		storageChg = 0.0 - storage;
		storage    = 0.0;
		outDisch   = inDischCurrent - storageChg / dt;
	}

	MFVarSetFloat (_MDOutRouting_Discharge0ID,   itemID, inDischCurrent);
	MFVarSetFloat (_MDOutRouting_Discharge1ID,   itemID, outDisch);
	MFVarSetFloat (_MDOutRouting_DischLevel3ID,  itemID, outDisch);
	MFVarSetFloat (_MDOutRouting_RiverStorChgID, itemID, storageChg);
	MFVarSetFloat (_MDOutRouting_RiverStorageID, itemID, storage);
}

int MDRouting_DischargeInChannelMuskingumDef () {

	if (_MDOutRouting_DischLevel3ID != MFUnset) return (_MDOutRouting_DischLevel3ID);

	MFDefEntering ("Discharge Routing - Muskingum");

	if (((_MDInCore_RunoffVolumeID     = MDCore_RunoffVolumeDef()) == CMfailed) ||
        ((_MDInRouting_MuskingumC0ID   = MDRouting_DischargeInChannelMuskingumCoeffDef()) == CMfailed) ||
        ((_MDInRouting_MuskingumC1ID   = MFVarGetID (MDVarRouting_MuskingumC1,     MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInRouting_MuskingumC2ID   = MFVarGetID (MDVarRouting_MuskingumC2,     MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInRouting_DischargeID     = MFVarGetID (MDVarRouting_Discharge,       "m3/s",   MFInput,  MFState, MFInitial))  == CMfailed) ||
        ((_MDOutRouting_Discharge0ID   = MFVarGetID (MDVarRouting_Discharge0,      "m3/s",   MFOutput, MFState, MFInitial))  == CMfailed) ||
        ((_MDOutRouting_Discharge1ID   = MFVarGetID (MDVarRouting_Discharge1,      "m3/s",   MFOutput, MFState, MFInitial))  == CMfailed) ||
        ((_MDOutRouting_RiverStorChgID = MFVarGetID (MDVarRouting_RiverStorageChg, "m3",     MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutRouting_RiverStorageID = MFVarGetID (MDVarRouting_RiverStorage,    "m3",     MFOutput, MFState, MFInitial))  == CMfailed) ||
        ((_MDOutRouting_DischLevel3ID  = MFVarGetID ("__DischLevel3",              "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction(_MDDischLevel3Muskingum) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Discharge Routing - Muskingum");
	return (_MDOutRouting_DischLevel3ID);
}

