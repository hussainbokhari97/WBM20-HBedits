/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDReservoir_Operation.c

dominik.wisser@unh.edu

Updated with a residence time dependent function 
Feb. 2009 DW

*******************************************************************************/

#include <MF.h>
#include <MD.h>
 
// Input
static int _MDInRouting_DischargeID       = MFUnset;
static int _MDInAux_MeanDischargeID       = MFUnset;
static int _MDInResCapacityID     = MFUnset;

// Output
static int _MDOutResStorageID        = MFUnset;
static int _MDOutResStorageChgID     = MFUnset;
static int _MDOutResReleaseID        = MFUnset;


static void _MDReservoir (int itemID) {

// Input
	float discharge;      // Current discharge [m3/s]
	float meanDischarge;  // Long-term mean annual discharge [m3/s]
	float resCapacity;    // Reservoir capacity [km3]
// Output
	float resStorage;     // Reservoir storage [km3]
	float resStorageChg;  // Reservoir storage change [km3/dt]
	float resRelease;     // Reservoir release [m3/s] 
// local
	float prevResStorage; // Reservoir storage from the previous time step [km3]
	float dt;             // Time step length [s]
	float balance;		  // water balance [m3/s]
// Parameters
	float drySeasonPct = 0.60;	// RJS 071511
	float wetSeasonPct = 0.16;	// RJS 071511
	float year = 0;				// RJS 082311
	
	discharge     = MFVarGetFloat (_MDInRouting_DischargeID,    itemID, 0.0);
	meanDischarge = MFVarGetFloat (_MDInAux_MeanDischargeID,    itemID, discharge);
	year 		  = MFDateGetCurrentYear();

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) <= 0.0) { 
		MFVarSetFloat (_MDOutResStorageID,    itemID, 0.0); 
		MFVarSetFloat (_MDOutResStorageChgID, itemID, 0.0); 
		MFVarSetFloat (_MDOutResReleaseID,    itemID, discharge);
		return;
	}


//	beta = resCapacity /(meanDischarge * 3600 * 24 * 365/1e9);
	dt = MFModelGet_dt ();
	prevResStorage = MFVarGetFloat(_MDOutResStorageID, itemID, 0.0);

	resRelease = discharge > meanDischarge ?
					wetSeasonPct * discharge  :
	 				drySeasonPct * discharge + (meanDischarge - discharge);

 	resStorage = prevResStorage + (discharge - resRelease) * 86400.0 / 1e9;

	if (resStorage > resCapacity) {
		resRelease = discharge * dt / 1e9 + prevResStorage - resCapacity;
		resRelease = resRelease * 1e9 / dt;
		resStorage = resCapacity;
	}
	else if (resStorage < 0.0) {
		resRelease = prevResStorage + discharge  * dt / 1e9;
		resRelease = resRelease * 1e9 / dt;
		resStorage=0;
	}
			
	resStorageChg = resStorage - prevResStorage;

	balance = discharge - resRelease - (resStorageChg / 86400 * 1e9);	// water balance

	MFVarSetFloat (_MDOutResStorageID,    itemID, resStorage);
	MFVarSetFloat (_MDOutResStorageChgID, itemID, resStorageChg);
	MFVarSetFloat (_MDOutResReleaseID,    itemID, resRelease);
}

int MDReservoir_OperationDef () {
	int optID = MFoff;
	const char *optStr;

	if (_MDOutResReleaseID != MFUnset) return (_MDOutResReleaseID);

	MFDefEntering ("Reservoirs");
	if ((optStr = MFOptionGet (MDOptConfig_Reservoirs)) != (char *) NULL) optID = CMoptLookup (MFswitchOptions, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFswitchOptions); return (CMfailed);
		case MFhelp: MFOptionMessage (MDOptConfig_Reservoirs, optStr, MFswitchOptions);
		case MFoff: break;
		case MFon:
			if (((_MDInAux_MeanDischargeID = MDAux_MeanDiscargehDef ())    == CMfailed) ||
                ((_MDInRouting_DischargeID = MDRouting_DischargeUptake ()) == CMfailed) ||
                ((_MDInResCapacityID       = MFVarGetID (MDVarReservoir_Capacity,      "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID       = MFVarGetID (MDVarReservoir_Storage,       "km3",  MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID    = MFVarGetID (MDVarReservoir_StorageChange, "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID       = MFVarGetID (MDVarReservoir_Release,       "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoir) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Reservoirs");
	return (_MDOutResReleaseID); 
}
