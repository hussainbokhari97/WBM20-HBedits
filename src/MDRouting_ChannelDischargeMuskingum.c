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
static int _MDInCore_RunoffFlowID     = MFUnset;
static int _MDInRouting_MuskingumC0ID = MFUnset;
static int _MDInRouting_MuskingumC1ID = MFUnset;
static int _MDInRouting_MuskingumC2ID = MFUnset;
static int _MDInRouting_DischargeID   = MFUnset;
// Output
static int _MDOutRouting_Discharge0ID   = MFUnset;
static int _MDOutRouting_Discharge1ID   = MFUnset;
static int _MDOutRouting_DischargeIntID = MFUnset;
static int _MDOutRouting_RiverStorChgID = MFUnset;
static int _MDOutRouting_RiverStorageID = MFUnset;

static void _MDDischLevel3Muskingum (int itemID) {
// Input
	float C0         = MFVarGetFloat (_MDInRouting_MuskingumC0ID, itemID, 1.0); // Muskingum C0 coefficient (current inflow)
	float C1         = MFVarGetFloat (_MDInRouting_MuskingumC1ID, itemID, 0.0); // Muskingum C1 coefficient (previous inflow)
	float C2         = MFVarGetFloat (_MDInRouting_MuskingumC2ID, itemID, 0.0); // MUskingum C2 coefficient (previous outflow) 
	float runoffFlow = MFVarGetFloat (_MDInCore_RunoffFlowID,     itemID, 0.0); // Runoff flow [m3/s]
// Initial
	float inDischPrevious = MFVarGetFloat (_MDOutRouting_Discharge0ID,   itemID, 0.0); // Upstream discharge at the previous time step [m3/s]
	float outDisch        = MFVarGetFloat (_MDOutRouting_Discharge1ID,   itemID, 0.0); // Downstream discharge [m3/s]
	float storage         = MFVarGetFloat (_MDOutRouting_RiverStorageID, itemID, 0.0); // River Storage [m3]
// Route
	float inDischCurrent  = MFVarGetFloat (_MDInRouting_DischargeID,     itemID, 0.0); // Upstream discharge at the current time step [m3/s]
// Output
	float storageChg;      // River Storage Change [m3]
// Local
	float dt = MFModelGet_dt ();

	inDischCurrent += runoffFlow;
	// negative C1 and C2 could cause negative discharge
	outDisch = MDMaximum (C0 * inDischCurrent + C1 * inDischPrevious + C2 * outDisch, 0.0);

	storageChg  = (inDischCurrent - outDisch) * dt;
	if (storage + storageChg > 0.0) storage += storageChg;
	else {
		storageChg = 0.0 - storage;
		storage    = 0.0;
		outDisch   = inDischCurrent > storageChg / dt ? inDischCurrent - storageChg / dt : 0.0;
	}
	MFVarSetFloat (_MDOutRouting_Discharge0ID,   itemID, inDischCurrent);
	MFVarSetFloat (_MDOutRouting_Discharge1ID,   itemID, outDisch);
	MFVarSetFloat (_MDOutRouting_DischargeIntID, itemID, outDisch);
	MFVarSetFloat (_MDOutRouting_RiverStorChgID, itemID, storageChg);
	MFVarSetFloat (_MDOutRouting_RiverStorageID, itemID, storage);
}

int MDRouting_ChannelDischargeMuskingumDef () {

	if (_MDOutRouting_DischargeIntID != MFUnset) return (_MDOutRouting_DischargeIntID);

	MFDefEntering ("Discharge Routing - Muskingum");

	if (((_MDInCore_RunoffFlowID       = MDCore_RunoffFlowDef())                        == CMfailed) ||
        ((_MDInRouting_MuskingumC0ID   = MDRouting_ChannelDischargeMuskingumCoeffDef()) == CMfailed) ||
        ((_MDInRouting_MuskingumC1ID   = MFVarGetID (MDVarRouting_MuskingumC1,     MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInRouting_MuskingumC2ID   = MFVarGetID (MDVarRouting_MuskingumC2,     MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInRouting_DischargeID     = MFVarGetID (MDVarRouting_Discharge,       "m3/s",   MFRoute,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutRouting_Discharge0ID   = MFVarGetID (MDVarRouting_Discharge0,      "m3/s",   MFOutput, MFState, MFInitial))  == CMfailed) ||
        ((_MDOutRouting_Discharge1ID   = MFVarGetID (MDVarRouting_Discharge1,      "m3/s",   MFOutput, MFState, MFInitial))  == CMfailed) ||
        ((_MDOutRouting_RiverStorChgID = MFVarGetID (MDVarRouting_RiverStorageChg, "m3",     MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutRouting_RiverStorageID = MFVarGetID (MDVarRouting_RiverStorage,    "m3",     MFOutput, MFState, MFInitial))  == CMfailed) ||
        ((_MDOutRouting_DischargeIntID = MFVarGetID ("__DischargeInternal",        "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction(_MDDischLevel3Muskingum) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Discharge Routing - Muskingum");
	return (_MDOutRouting_DischargeIntID);
}

