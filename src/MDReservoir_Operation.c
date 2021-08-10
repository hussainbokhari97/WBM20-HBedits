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
static int _MDInResCapacityID           = MFUnset;
static int _MDInResUptakeID             = MFUnset;

// Output
static int _MDOutResStorageID           = MFUnset;
static int _MDOutResStorageChgID        = MFUnset;
static int _MDOutResReleaseID           = MFUnset;
static int _MDOutResConsumableReleaseID = MFUnset;


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
// local
	float prevResStorage; // Reservoir storage from the previous time step [km3]
	float dt;             // Time step length [s]
// Parameters
	float drySeasonPct = 0.60; // RJS 071511
	float wetSeasonPct = 0.16; // RJS 071511
	float year;                // RJS 082311
	
	discharge     = MFVarGetFloat (_MDInRouting_DischargeID,    itemID, 0.0);
	meanDischarge = MFVarGetFloat (_MDInAux_MeanDischargeID,    itemID, discharge);
	year 		  = MFDateGetCurrentYear();

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) <= 0.0) { 
		MFVarSetFloat (_MDOutResStorageID,    itemID, 0.0); 
		MFVarSetFloat (_MDOutResStorageChgID, itemID, 0.0); 
		MFVarSetFloat (_MDOutResReleaseID,    itemID, discharge);
		return;
	}
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
			
	resStorageChg = resStorage - prevResStorage;

	MFVarSetFloat (_MDOutResStorageID,           itemID, resStorage);
	MFVarSetFloat (_MDOutResStorageChgID,        itemID, resStorageChg);
	MFVarSetFloat (_MDOutResReleaseID,           itemID, resRelease);
	MFVarSetFloat (_MDOutResConsumableReleaseID, itemID, resRelease > discharge ? resRelease - discharge : 0.0);
}

static void _MDReservoirGMLC (int itemID) {

}

enum { MDhelp, MDwisser, MDgmlc };

int MDReservoir_OperationDef () {
	int optID = MDwisser;
	const char *optStr, *options [ ] = { MFhelpStr, "Wisser", "gmlc" };

	if (_MDOutResReleaseID != MFUnset) return (_MDOutResReleaseID);

	MFDefEntering ("Reservoirs");
	if ((optStr = MFOptionGet (MDVarReservoir_Release)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage (MDOptConfig_Reservoirs, optStr, options); return (CMfailed);
		case MDhelp: MFOptionMessage (MDOptConfig_Reservoirs, optStr, options);
		case MDwisser:
			if (((_MDInAux_MeanDischargeID     = MDAux_MeanDischargeDef    ()) == CMfailed) ||
				((_MDInResUptakeID             = MDReservoir_UptakeDef     ()) == CMfailed) ||
                ((_MDInRouting_DischargeID     = MDRouting_DischargeUptake ()) == CMfailed) ||
            	((_MDInResCapacityID           = MFVarGetID (MDVarReservoir_Capacity,          "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID           = MFVarGetID (MDVarReservoir_Storage,           "km3",  MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID        = MFVarGetID (MDVarReservoir_StorageChange,     "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID           = MFVarGetID (MDVarReservoir_Release,           "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
 			    ((_MDOutResConsumableReleaseID = MFVarGetID (MDVarReservoir_ConsumableRelease, "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirWisser) == CMfailed)) return (CMfailed);
			break;
		case MDgmlc: break;
	}
	MFDefLeaving ("Reservoirs");
	return (_MDOutResReleaseID); 
}
