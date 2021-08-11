/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDReservoir_Operation.c

bfekete@gc.cuny.edu

******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInRouting_DischargeID      = MFUnset;
static int _MDInAux_MeanDischargeID      = MFUnset;
static int _MDInResTargetHighFlowID      = MFUnset;
static int _MDInResTargetLowFlowID       = MFUnset;
static int _MDInResCapacityID            = MFUnset;
static int _MDInResUptakeID              = MFUnset;

// Output
static int _MDOutResStorageID            = MFUnset;
static int _MDOutResStorageChgID         = MFUnset;
static int _MDOutResReleaseID            = MFUnset;
static int _MDOutResExtractableReleaseID = MFUnset;

static void _MDReservoirWisser (int itemID) {

// Input
	float discharge;      // Current discharge [m3/s]
	float meanDischarge;  // Long-term mean annual discharge [m3/s]
	float resCapacity;    // Reservoir capacity [km3]
	float resUptake;      // Water uptake from reservoir [m3/s]
// Output
	float resStorage;     // Reservoir storage [km3]
	float resStorageChg;  // Reservoir storage change [km3/dt]
	float resRelease;     // Reservoir release [m3/s] 
	float resExtRelease;  // Reservoir extractable release [m3/s]
// local
	float prevResStorage; // Reservoir storage from the previous time step [km3]
	float dt;             // Time step length [s]
// Parameters
	float drySeasonPct = 0.60; // RJS 071511
	float wetSeasonPct = 0.16; // RJS 071511
	
	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) <= 0.0) { 
		MFVarSetFloat (_MDOutResStorageID,            itemID, 0.0); 
		MFVarSetFloat (_MDOutResStorageChgID,         itemID, 0.0); 
		MFVarSetFloat (_MDOutResReleaseID,            itemID, discharge);
		MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, 0.0);
		return;
	}

	discharge     = MFVarGetFloat (_MDInRouting_DischargeID,      itemID, 0.0);
	meanDischarge = MFVarGetFloat (_MDInAux_MeanDischargeID,      itemID, discharge);
	resExtRelease = MFVarGetFloat (_MDOutResExtractableReleaseID, itemID, 0.0);
	resUptake = _MDInResUptakeID != MFUnset ? MFVarGetFloat (_MDInResUptakeID,itemID, 0.0) : 0.0; 

//	beta = resCapacity /(meanDischarge * 3600 * 24 * 365/1e9);
	dt = MFModelGet_dt ();
	prevResStorage = MFVarGetFloat(_MDOutResStorageID, itemID, 0.0);

	resRelease = discharge > meanDischarge ? wetSeasonPct * discharge : drySeasonPct * discharge  + (meanDischarge - discharge);

 	resStorage = prevResStorage + (discharge - resRelease - resUptake) * dt / 1e9;

	if (resStorage > resCapacity) {
		resRelease = (discharge - resUptake) * dt / 1e9 + prevResStorage - resCapacity;
		resRelease = resRelease * 1e9 / dt;
		resStorage = resCapacity;
	}
	else if (resStorage < 0.0) {
		resRelease = prevResStorage + (discharge + resUptake) * dt / 1e9;
		resRelease = resRelease * 1e9 / dt;
		resStorage = 0.0;
	}
			
	resStorageChg  = resStorage - prevResStorage;
	resExtRelease += resRelease > discharge ? resRelease - discharge : 0.0;
	MFVarSetFloat (_MDOutResStorageID,            itemID, resStorage);
	MFVarSetFloat (_MDOutResStorageChgID,         itemID, resStorageChg);
	MFVarSetFloat (_MDOutResReleaseID,            itemID, resRelease);
	MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, resExtRelease);
}

static void _MDReservoirOptimized (int itemID) {
// Input
	float discharge;       // Current discharge [m3/s]
	float targetLowFlow;   // Target low flow [m3/s]
	float targetHighFlow;  // Target high flow [m3/s]
	float resUptake;       // Water uptake from reservoir [m3/s]
	float resCapacity;     // Reservoir capacity [km3]
// Output
	float resStorage;      // Reservoir storage [km3]
	float resStorageChg;   // Reservoir storage change [km3/dt]
	float resRelease;      // Reservoir release [m3/s] 
	float resExtRelease;   // Reservoir extractable release [m3/s]
// Local
	float prevResStorage;  // Previous reservoir storage [km3]
	float dt;              // Time step length [s]

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) <= 0.0) { 
		MFVarSetFloat (_MDOutResStorageID,            itemID, 0.0); 
		MFVarSetFloat (_MDOutResStorageChgID,         itemID, 0.0); 
		MFVarSetFloat (_MDOutResReleaseID,            itemID, discharge);
		MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, 0.0);
		return;
	}
	             dt = MFModelGet_dt ();
	 discharge      = MFVarGetFloat (_MDInRouting_DischargeID,   itemID, 0.0);
	 prevResStorage = MFVarGetFloat (_MDOutResStorageID,         itemID, 0.0);
	 targetLowFlow  = _MDInResTargetLowFlowID  != MFUnset ? MFVarGetFloat (_MDInResTargetLowFlowID,  itemID, discharge) : discharge;
	 targetHighFlow = _MDInResTargetHighFlowID != MFUnset ? MFVarGetFloat (_MDInResTargetHighFlowID, itemID, discharge) : discharge;

	if (targetLowFlow > discharge) {
	   	resRelease = (targetLowFlow - discharge) * dt / 1e9 < prevResStorage ? targetLowFlow : discharge + prevResStorage * 1e9 / dt;
	}
	if (discharge - targetHighFlow) {
		resRelease = (discharge - targetHighFlow) * dt / 1e9 < (resCapacity - prevResStorage) ? targetLowFlow : discharge - (resCapacity - prevResStorage) * 1e9 / dt;
	}
	resStorage = prevResStorage + (discharge - resRelease) * dt / 1e9;

	MFVarSetFloat (_MDOutResStorageID,            itemID, resStorage); 
	MFVarSetFloat (_MDOutResStorageChgID,         itemID, resStorageChg); 
	MFVarSetFloat (_MDOutResReleaseID,            itemID, resRelease);
	MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, resRelease > discharge ? resRelease - discharge : 0.0);
}

static void _MDReservoirGMLC (int itemID) {

}

enum { MDhelp, MDwisser, MDoptimized, MDgmlc };

int MDReservoir_OperationDef () {
	int optID = MDwisser;
	const char *optStr, *options [ ] = { MFhelpStr, "Wisser", "optimized" "gmlc" , (char *) NULL };

	if (_MDOutResReleaseID != MFUnset) return (_MDOutResReleaseID);

	MFDefEntering ("Reservoirs");
	if ((optStr = MFOptionGet (MDVarReservoir_Release)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage (MDOptConfig_Reservoirs, optStr, options); return (CMfailed);
		case MDhelp: MFOptionMessage (MDOptConfig_Reservoirs, optStr, options);
		case MDwisser:
			if (((_MDInRouting_DischargeID      = MDRouting_DischargeUptake     ()) == CMfailed) ||
				((_MDInResUptakeID              = MDReservoir_UptakeDef         ()) == CMfailed) ||
				((_MDInAux_MeanDischargeID      = MDAux_MeanDischargeDef        ()) == CMfailed) ||
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID            = MFVarGetID (MDVarReservoir_Storage,            "km3",  MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID         = MFVarGetID (MDVarReservoir_StorageChange,      "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID            = MFVarGetID (MDVarReservoir_Release,            "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
 			    ((_MDOutResExtractableReleaseID = MFVarGetID (MDVarReservoir_ExtractableRelease, "m3/s", MFRoute,  MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirWisser) == CMfailed)) return (CMfailed);
			break;
		case MDoptimized: break;
			if (((_MDInRouting_DischargeID      = MDRouting_DischargeUptake     ()) == CMfailed) ||
				((_MDInResUptakeID              = MDReservoir_UptakeDef         ()) == CMfailed) ||
				((_MDInResTargetLowFlowID       = MDReservoir_TargetLowFlowDef  ()) == CMfailed) ||
				((_MDInResTargetHighFlowID      = MDReservoir_TargetHighFlowDef ()) == CMfailed) ||                
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID            = MFVarGetID (MDVarReservoir_Storage,            "km3",  MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID         = MFVarGetID (MDVarReservoir_StorageChange,      "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID            = MFVarGetID (MDVarReservoir_Release,            "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
 			    ((_MDOutResExtractableReleaseID = MFVarGetID (MDVarReservoir_ExtractableRelease, "m3/s", MFRoute,  MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirOptimized) == CMfailed)) return (CMfailed);
		case MDgmlc:       break;
	}
	MFDefLeaving ("Reservoirs");
	return (_MDOutResReleaseID); 
}
