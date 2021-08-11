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
static int _MDInRouting_DischargeID     = MFUnset;
static int _MDInAux_MeanDischargeID     = MFUnset;
static int _MDInAux_MinDischargeID      = MFUnset;
static int _MDInResCapacityID           = MFUnset;
static int _MDInResUptakeID             = MFUnset;

// Output
static int _MDOutResStorageID           = MFUnset;
static int _MDOutResStorageChgID        = MFUnset;
static int _MDOutResDeficitID           = MFUnset;
static int _MDOutResAccumDeficitID      = MFUnset;
static int _MDOutResMaxAccumDeficitID   = MFUnset;
static int _MDOutResReleaseID           = MFUnset;
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

static void _MDReservoirMinDeficit (int itemID) {
// Input
	float discharge;       // Current discharge [m3/s]
	float meanDischarge;   // Long-term mean annual discharge [m3/s]
	float minDischarge;    // Long-term minimum discharge [m3/s]
	float resCapacity;     // Reservoir capacity [km3]
	float resUptake;       // Water uptake from reservoir [m3/s]
// Output
	float resStorage;      // Reservoir storage [km3]
	float resStorageChg;   // Reservoir storage change [km3/dt]
	float resRelease;      // Reservoir release [m3/s] 
	float resExtRelease;   // Reservoir extractable release [m3/s]
	float deficit;         // Inflow deficit (bellow mean annual discharge [m3/s]
	float accumDeficit;    // Accumulated deficit during the season [m3/s]
	float maxAccumDeficit; // Maximum accumulated deficit [m3/s]
// Local
	float targetRelease;   // Targeted minimum flow [m3/s]
	float optResCapacity;  // Reservoir capacity required for fully eliminating low flows;
	float prevResStorage;  // Reservoir storage from the previous time step [km3]
	float surplus;         // Reservoir recharge [m3/s]
	float dt;              // Time step length [s]

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) <= 0.0) { 
		MFVarSetFloat (_MDOutResStorageID,            itemID, 0.0); 
		MFVarSetFloat (_MDOutResStorageChgID,         itemID, 0.0); 
		MFVarSetFloat (_MDOutResReleaseID,            itemID, discharge);
		MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, 0.0);
		MFVarSetFloat (_MDOutResDeficitID,            itemID, 0.0);
		MFVarSetFloat (_MDOutResAccumDeficitID,       itemID, 0.0);
		MFVarSetFloat (_MDOutResMaxAccumDeficitID,    itemID, 0.0);
		return;
	}
	             dt = MFModelGet_dt ();
	 discharge      = MFVarGetFloat (_MDInRouting_DischargeID,   itemID, 0.0);
	 prevResStorage = MFVarGetFloat(_MDOutResStorageID, itemID, 0.0);
	 meanDischarge  = MFVarGetFloat (_MDInAux_MeanDischargeID,   itemID, discharge);
	  minDischarge  = MFVarGetFloat (_MDInAux_MinDischargeID,    itemID, discharge);
	maxAccumDeficit = MFVarGetFloat (_MDOutResMaxAccumDeficitID, itemID, discharge);

	 optResCapacity = maxAccumDeficit * dt / 1e9;
	  targetRelease = meanDischarge > 0.0 ? meanDischarge * (1 - minDischarge / meanDischarge) * resCapacity / (optResCapacity > resCapacity ? optResCapacity : resCapacity): 0.0;

	if (discharge < targetRelease) {
		       deficit  = discharge - minDischarge;
		   	 resRelease = targetRelease * dt / 1e9 < prevResStorage ? targetRelease : prevResStorage * 1e9 / dt;
		     resStorage = prevResStorage - resRelease * dt / 1e9;
	}
	else {
			deficit = 0.0;
		    surplus = (discharge - targetRelease) * dt / 1e9;
		resStorage += resCapacity - prevResStorage < surplus ? resCapacity - prevResStorage : surplus;
	}
	  resStorageChg = resStorage - prevResStorage;
	  resRelease    = discharge  - resStorageChg * 1e9 / dt;
	  accumDeficit += MFDateGetDayOfYear () > 0 ? deficit : 0.0;
	maxAccumDeficit = accumDeficit > maxAccumDeficit ? accumDeficit : maxAccumDeficit;
	MFVarSetFloat (_MDOutResStorageID,            itemID, resStorage); 
	MFVarSetFloat (_MDOutResStorageChgID,         itemID, resStorageChg); 
	MFVarSetFloat (_MDOutResReleaseID,            itemID, resRelease);
	MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, resRelease > discharge ? resRelease - discharge : 0.0);
	MFVarSetFloat (_MDOutResDeficitID,            itemID, deficit);
	MFVarSetFloat (_MDOutResAccumDeficitID,       itemID, accumDeficit);
	MFVarSetFloat (_MDOutResMaxAccumDeficitID,    itemID, maxAccumDeficit);
}

static void _MDReservoirGMLC (int itemID) {

}

enum { MDhelp, MDwisser, MDminDeficit, MDgmlc };

int MDReservoir_OperationDef () {
	int optID = MDwisser;
	const char *optStr, *options [ ] = { MFhelpStr, "Wisser", "MinDeficit" "gmlc" };

	if (_MDOutResReleaseID != MFUnset) return (_MDOutResReleaseID);

	MFDefEntering ("Reservoirs");
	if ((optStr = MFOptionGet (MDVarReservoir_Release)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage (MDOptConfig_Reservoirs, optStr, options); return (CMfailed);
		case MDhelp: MFOptionMessage (MDOptConfig_Reservoirs, optStr, options);
		case MDwisser:
			if (((_MDInAux_MeanDischargeID      = MDAux_MeanDischargeDef    ()) == CMfailed) ||
				((_MDInResUptakeID              = MDReservoir_UptakeDef     ()) == CMfailed) ||
                ((_MDInRouting_DischargeID      = MDRouting_DischargeUptake ()) == CMfailed) ||
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID            = MFVarGetID (MDVarReservoir_Storage,            "km3",  MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID         = MFVarGetID (MDVarReservoir_StorageChange,      "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID            = MFVarGetID (MDVarReservoir_Release,            "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
 			    ((_MDOutResExtractableReleaseID = MFVarGetID (MDVarReservoir_ExtractableRelease, "m3/s", MFRoute,  MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirWisser) == CMfailed)) return (CMfailed);
			break;
		case MDminDeficit: break;
			if (((_MDInAux_MeanDischargeID      = MDAux_MeanDischargeDef    ()) == CMfailed) ||
			    ((_MDInAux_MinDischargeID       = MDAux_MinimumDischargeDef ()) == CMfailed) ||
				((_MDInResUptakeID              = MDReservoir_UptakeDef     ()) == CMfailed) ||
                ((_MDInRouting_DischargeID      = MDRouting_DischargeUptake ()) == CMfailed) ||
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID            = MFVarGetID (MDVarReservoir_Storage,            "km3",  MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID         = MFVarGetID (MDVarReservoir_StorageChange,      "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID            = MFVarGetID (MDVarReservoir_Release,            "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
 			    ((_MDOutResExtractableReleaseID = MFVarGetID (MDVarReservoir_ExtractableRelease, "m3/s", MFRoute,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResDeficitID            = MFVarGetID (MDVarReservoir_Deficit,            "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResAccumDeficitID       = MFVarGetID (MDVarReservoir_AccumDeficit,       "m3/s", MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResMaxAccumDeficitID    = MFVarGetID (MDVarReservoir_MaxAccumDeficit,    "m3/s", MFOutput, MFState, MFInitial))  == CMfailed) ||
                (MFModelAddFunction (_MDReservoirMinDeficit) == CMfailed)) return (CMfailed);
		case MDgmlc:       break;
	}
	MFDefLeaving ("Reservoirs");
	return (_MDOutResReleaseID); 
}
