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

static int _MDInGarbageNonIrrUptake      = MFUnset;
static int _MDInGarbageNonIrrMeanUptake  = MFUnset;
static int _MDInGarbageIrrUptake         = MFUnset;
static int _MDInGarbageIrrMeanUptake     = MFUnset;

// Output
static int _MDOutResStorageID            = MFUnset;
static int _MDOutResStorageChgID         = MFUnset;
static int _MDOutResReleaseID            = MFUnset;
static int _MDOutResExtractableReleaseID = MFUnset;

static void _MDReservoirWisser (int itemID) {

// Input
	float discharge;           // Current discharge [m3/s]
	float meanDischarge;       // Long-term mean annual discharge [m3/s]
	float resCapacity;         // Reservoir capacity [km3]
	float resUptake;           // Water uptake from reservoir [m3/s]
// Output
	float resStorage    = 0.0; // Reservoir storage [km3]
	float resStorageChg = 0.0; // Reservoir storage change [km3/dt]
	float resRelease;          // Reservoir release [m3/s] 
	float resExtRelease;       // Reservoir extractable release [m3/s]
// local
	float prevResStorage;      // Reservoir storage from the previous time step [km3]
	float dt;                  // Time step length [s]
// Parameters
	float drySeasonPct = 0.60; // RJS 071511
	float wetSeasonPct = 0.16; // RJS 071511

	resRelease    =	
	discharge     = MFVarGetFloat (_MDInRouting_DischargeID,      itemID, 0.0);
	resExtRelease = MFVarGetFloat (_MDOutResExtractableReleaseID, itemID, 0.0);

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) > 0.0) { 
		            dt = MFModelGet_dt ();
		 meanDischarge = MFVarGetFloat (_MDInAux_MeanDischargeID,      itemID, discharge);
		     resUptake = _MDInResUptakeID != MFUnset ? MFVarGetFloat (_MDInResUptakeID,itemID, 0.0) : 0.0; 
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
		resExtRelease  = resRelease > discharge ? resRelease - discharge : 0.0;
		resExtRelease  = 0.0;
	}
	MFVarSetFloat (_MDOutResStorageID,            itemID, resStorage);
	MFVarSetFloat (_MDOutResStorageChgID,         itemID, resStorageChg);
	MFVarSetFloat (_MDOutResReleaseID,            itemID, resRelease);
	MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, resExtRelease);
}

static void _MDReservoirOptimized (int itemID) {
// Input
	float discharge;           // Current discharge [m3/s]
	float targetLowFlow;       // Target low flow [m3/s]
	float targetHighFlow;      // Target high flow [m3/s]
	float resUptake;           // Water uptake from reservoir [m3/s]
	float resCapacity;         // Reservoir capacity [km3]
// Output
	float resStorage    = 0.0; // Reservoir storage [km3]
	float resStorageChg = 0.0; // Reservoir storage change [km3/dt]
	float resRelease;          // Reservoir release [m3/s] 
	float resExtRelease;       // Reservoir extractable release [m3/s]
// Local
	float prevResStorage;      // Previous reservoir storage [km3]
	float dt;                  // Time step length [s]

	resRelease     =
	discharge      = MFVarGetFloat (_MDInRouting_DischargeID,      itemID, 0.0);
	resExtRelease  = MFVarGetFloat (_MDOutResExtractableReleaseID, itemID, 0.0);

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) > 0.0) {
		            dt = MFModelGet_dt ();
		prevResStorage = MFVarGetFloat (_MDOutResStorageID,            itemID, 0.0);
		targetLowFlow  = _MDInResTargetLowFlowID  != MFUnset ? MFVarGetFloat (_MDInResTargetLowFlowID,  itemID, discharge) : discharge;
		targetHighFlow = _MDInResTargetHighFlowID != MFUnset ? MFVarGetFloat (_MDInResTargetHighFlowID, itemID, discharge) : discharge;

		if (targetLowFlow > discharge)
	   		resRelease = (targetLowFlow - discharge) * dt / 1e9 < prevResStorage ? targetLowFlow : discharge + prevResStorage * 1e9 / dt;
		else if (discharge - targetHighFlow)
			resRelease = (discharge - targetHighFlow) * dt / 1e9 < (resCapacity - prevResStorage) ? targetLowFlow : discharge - (resCapacity - prevResStorage) * 1e9 / dt;
		else
			resRelease = discharge;

		resStorage    = prevResStorage + (discharge - resRelease) * dt / 1e9;
		resExtRelease = resRelease > discharge ? resRelease - discharge : 0.0;
	}
	MFVarSetFloat (_MDOutResStorageID,            itemID, resStorage); 
	MFVarSetFloat (_MDOutResStorageChgID,         itemID, resStorageChg); 
	MFVarSetFloat (_MDOutResReleaseID,            itemID, resRelease);
	MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, resExtRelease);
}

static void _MDReservoirGarbage (int itemID) {

}

enum { MDhelp, MDwisser, MDoptimized, MDgarbage };

int MDReservoir_OperationDef () {
	int optID = MDwisser;
	const char *optStr, *options [ ] = { MFhelpStr, "Wisser", "optimized" "garbage" , (char *) NULL };

	if (_MDOutResReleaseID != MFUnset) return (_MDOutResReleaseID);

	MFDefEntering ("Reservoirs");
	if ((optStr = MFOptionGet (MDVarReservoir_Release)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage (MDOptConfig_Reservoirs, optStr, options); return (CMfailed);
		case MDhelp: MFOptionMessage (MDOptConfig_Reservoirs, optStr, options);
		case MDwisser:
			if (((_MDInRouting_DischargeID      = MDRouting_DischargeUptakeDef  ()) == CMfailed) ||
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
			if (((_MDInRouting_DischargeID      = MDRouting_DischargeUptakeDef  ()) == CMfailed) ||
				((_MDInResUptakeID              = MDReservoir_UptakeDef         ()) == CMfailed) ||
				((_MDInResTargetLowFlowID       = MDReservoir_TargetLowFlowDef  ()) == CMfailed) ||
				((_MDInResTargetHighFlowID      = MDReservoir_TargetHighFlowDef ()) == CMfailed) ||                
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID            = MFVarGetID (MDVarReservoir_Storage,            "km3",  MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID         = MFVarGetID (MDVarReservoir_StorageChange,      "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID            = MFVarGetID (MDVarReservoir_Release,            "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
 			    ((_MDOutResExtractableReleaseID = MFVarGetID (MDVarReservoir_ExtractableRelease, "m3/s", MFRoute,  MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirOptimized) == CMfailed)) return (CMfailed);
		case MDgarbage:
			if (((_MDInRouting_DischargeID      = MDRouting_DischargeUptakeDef  ()) == CMfailed) ||
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID            = MFVarGetID (MDVarReservoir_Storage,            "km3",  MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID         = MFVarGetID (MDVarReservoir_StorageChange,      "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID            = MFVarGetID (MDVarReservoir_Release,            "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirGarbage) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Reservoirs");
	return (_MDOutResReleaseID); 
}
