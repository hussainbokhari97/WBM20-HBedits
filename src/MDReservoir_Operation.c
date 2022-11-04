/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDReservoir_Operation.c

bfekete@gc.cuny.edu

******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInRouting_DischargeID      = MFUnset;
static int _MDInAux_MeanDischargeID      = MFUnset;
static int _MDInResCapacityID            = MFUnset;

static int _MDInResUptakeID              = MFUnset;
static int _MDInResNatFlowMeanMonthlyID  = MFUnset;
static int _MDInResNatFlowMeanDailyID    = MFUnset;
static int _MDInResInitStorageID         = MFUnset;
static int _MDInResStorageRatioID        = MFUnset;
static int _MDInResStorageRatio25ID      = MFUnset;
static int _MDInResStorageRatio75ID      = MFUnset;
static int _MDInResDemandFactorID        = MFUnset;
static int _MDInResIncMultID             = MFUnset;
static int _MDInResIncrement1ID          = MFUnset;
static int _MDInResIncrement2ID          = MFUnset;
static int _MDInResIncrement3ID          = MFUnset;
static int _MDInResAlphaID               = MFUnset;
static int _MDInResReleaseAdjID          = MFUnset;
// Output
static int _MDOutResStorageID            = MFUnset;
static int _MDOutResStorageChgID         = MFUnset;
static int _MDOutResInflowID             = MFUnset;
static int _MDOutResReleaseID            = MFUnset;
static int _MDOutResExtractableReleaseID = MFUnset;
static int _MDOutResTargetReleaseID      = MFUnset; // for Debuging only
static int _MDOutResAttemptedReleaseID   = MFUnset; // for Debuging only
static int _MDOutResBottomReleaseID      = MFUnset;
static int _MDOutResSpillID              = MFUnset;

static void _MDReservoirWisser (int itemID) {

// Input
	float discharge;             // Current discharge [m3/s]
	float meanDischarge;         // Long-term mean annual discharge [m3/s]
	float resCapacity;           // Reservoir capacity [km3]
	float resUptake;             // Water uptake from reservoir [m3/s]
// Output
	float resStorage    = 0.0;   // Reservoir storage [km3]
	float resStorageChg = 0.0;   // Reservoir storage change [km3/dt]
	float resInflow;             // Reservoir release [m3/s] 
	float resRelease;            // Reservoir release [m3/s] 
	float resExtRelease;         // Reservoir extractable release [m3/s]
// local
	float prevResStorage;        // Reservoir storage from the previous time step [km3]
	float dt = MFModelGet_dt (); // Time step length [s]
// Parameters
	float drySeasonPct = 0.60; // RJS 071511
	float wetSeasonPct = 0.16; // RJS 071511

	resRelease    =	
	resInflow     =
	discharge     = MFVarGetFloat (_MDInRouting_DischargeID,      itemID, 0.0);
	resExtRelease = MFVarGetFloat (_MDOutResExtractableReleaseID, itemID, 0.0);

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) > 0.0001) { // TODO Arbitrary limit      
		 meanDischarge = MFVarGetFloat (_MDInAux_MeanDischargeID,      itemID, discharge);
		prevResStorage = MFVarGetFloat(_MDOutResStorageID, itemID, 0.0);
		     resUptake = _MDInResUptakeID != MFUnset ? MFVarGetFloat (_MDInResUptakeID,itemID, 0.0) : 0.0; 

		if (prevResStorage * 1e9 / dt < resUptake) {
			resUptake = prevResStorage * 1e9 / dt;
			prevResStorage = 0.0;
		}
		resRelease = discharge > meanDischarge ? wetSeasonPct * discharge : drySeasonPct * discharge  + (meanDischarge - discharge);

 		resStorage = prevResStorage + (discharge - resUptake - resRelease) * dt / 1e9;
		if (resStorage > resCapacity) {
			resRelease += (resStorage - resCapacity) * 1e9 / dt;
			resStorage  = resCapacity;
		}
		else if (resStorage < 0.0) {
			resRelease  = prevResStorage * 1e9 / dt + discharge;
			resStorage  = 0.0;
		}
		resStorageChg  = resStorage - prevResStorage;
		resExtRelease = resRelease > discharge ? resRelease - discharge + (resExtRelease < discharge ? resExtRelease : discharge) : 0.0;
	}
	MFVarSetFloat (_MDOutResStorageID,            itemID, resStorage);
	MFVarSetFloat (_MDOutResStorageChgID,         itemID, resStorageChg);
	MFVarSetFloat (_MDOutResInflowID,             itemID, resInflow);
	MFVarSetFloat (_MDOutResReleaseID,            itemID, resRelease);
	MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, resExtRelease);
}

static void _MDReservoirSNL (int itemID) {
	// Input
	float discharge;           // Current discharge [m3/s] -- this is the inflow to the dam
	float natFlowMeanMonthly;  // Naturalized long-term mean monthly inflow [m3/s]
	float natFlowMeanDaily;    // Naturalized long-term mean daily inflow [m3/s]
	float resCapacity;         // Reservoir capacity [km3]
	float resInitStorage;      // Reference storage dictatink actual release ratio [km3]
	float storageRatio;        // constant per dam hussain check   
	float storageRatio25;      // constant per dam hussain check
	float storageRatio75;      // constant per dam hussain check
	float demandFactor;        // monthly constant per dam
	float incMult;             // monthly constant per dam 
	float increment1;          // monthly constant per dam 
	float increment2;          // monthly constant per dam 
	float increment3;          // monthly constant per dam 
	float alpha;               // monthly constant per dam 
	float releaseAdj;          // monthly constant per dam 
	// Output
	float resStorage    = 0.0; // Reservoir storage [km3]
	float resStorageChg = 0.0; // Reservoir storage change [km3/dt]
	float resRelease;          // Reservoir release [m3/s] 
	float resExtRelease;       // Reservoir extractable release [m3/s]
	float releaseTarget = 0.0; // Target reservoir release [m3/s]
	// Local
	float resInflow;           // Reservoir inflow [m3/s] 
	float waterDemandMeanDaily;
	float prevResStorage;      // Previous reservoir storage [km3]
	float krls;                // release ratio
	float initial_krls;  
	float waterDemandMeanMonthly;
	float bottomRelease;
	float spill = 0.0;            // gets calculated further down, otherwise 0

	bottomRelease  =  // equals total release, unless there is Spill
	resRelease     =
	resInflow      =
	discharge      = MFVarGetFloat (_MDInRouting_DischargeID,      itemID, 0.0);
	resExtRelease  = MFVarGetFloat (_MDOutResExtractableReleaseID, itemID, 0.0);

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) > 0.0001) { // TODO Arbitrary limit!!!!
		float res_cap_25;
		float res_cap_75;
		float dt = MFModelGet_dt ();  // Time step length [s]
			prevResStorage = MFVarGetFloat (_MDOutResStorageID,           itemID, 0.0);
		natFlowMeanMonthly = MFVarGetFloat (_MDInResNatFlowMeanMonthlyID, itemID, 0.0);
		  natFlowMeanDaily = MFVarGetFloat (_MDInResNatFlowMeanDailyID,   itemID, 0.0);
    	    resInitStorage = MFVarGetFloat (_MDInResInitStorageID,        itemID, 0.0);
	          storageRatio = MFVarGetFloat (_MDInResStorageRatioID,       itemID, 0.0);
    	    storageRatio25 = MFVarGetFloat (_MDInResStorageRatio25ID,     itemID, 0.0);
    	    storageRatio75 = MFVarGetFloat (_MDInResStorageRatio75ID,     itemID, 0.0);
    	      demandFactor = MFVarGetFloat (_MDInResDemandFactorID,       itemID, 0.0);
    	           incMult = MFVarGetFloat (_MDInResIncMultID,            itemID, 0.0);
    	        increment1 = MFVarGetFloat (_MDInResIncrement1ID,         itemID, 0.0);
    	        increment2 = MFVarGetFloat (_MDInResIncrement2ID,         itemID, 0.0);
    	        increment3 = MFVarGetFloat (_MDInResIncrement3ID,         itemID, 0.0);
    	             alpha = MFVarGetFloat (_MDInResAlphaID,              itemID, 0.0);
			    releaseAdj = MFVarGetFloat (_MDInResReleaseAdjID,         itemID, 0.0);
		res_cap_25 = resStorage * storageRatio25;
		res_cap_75 = resStorage * storageRatio75;
		// ARIEL EDITED THIS TO INCULDE "{}" -- not sure it's needed, so please remove if not.
		if (resInitStorage > resCapacity) {resInitStorage = resCapacity; } // This could only happen before the model updates the initial storage
        
	// MAIN RULES begin ---->
		waterDemandMeanDaily   = demandFactor * natFlowMeanDaily;
		waterDemandMeanMonthly = demandFactor * natFlowMeanMonthly;                               
		// rough interpretation/goal for a release target    
		releaseTarget = natFlowMeanDaily + waterDemandMeanDaily - waterDemandMeanMonthly; 
		// krls before adjustment
		initial_krls = (resInitStorage / (alpha * resCapacity * storageRatio));
		// condition when storage is above 75% of normal or max
		if ((resInflow >= natFlowMeanMonthly) && (resInitStorage >= res_cap_75)) krls = (1 + incMult * increment1) * initial_krls;
		if ((resInflow <  natFlowMeanMonthly) && (resInitStorage >= res_cap_75)) krls = (1 + increment1) * initial_krls;
		// condition when storage is 25-75% of normal or max 
		if ((resInflow >= natFlowMeanMonthly) && (res_cap_75 > resInitStorage >= res_cap_25)) krls = (1 + incMult * increment2) * initial_krls;
		if ((resInflow <  natFlowMeanMonthly) && (res_cap_75 > resInitStorage >= res_cap_25)) krls = (1 + increment2) * initial_krls;
		// condition when storage is below 25% of normal or max 
		if ((resInflow >= natFlowMeanMonthly) && (resInitStorage < res_cap_25)) krls = (1 + incMult * increment3) * initial_krls;
		if ((resInflow <  natFlowMeanMonthly) && (resInitStorage < res_cap_25)) krls = (1 + increment3) * initial_krls;
		// adjustment to consider inflow on given day (should help with extremes)
		resRelease = 0.5 * ((krls * releaseTarget) + (releaseAdj * resInflow));
		// assume 5% envrionemntal flow minimum, and makes sure there is no negative flow.
		if (resRelease < 0.05 * resInflow) resRelease = resInflow * 0.05;
	// MAIN RULES finish
		resStorage = prevResStorage + (discharge - resRelease) * dt / 1e9;
		if (resStorage > resCapacity) {
			spill = (resStorage - resCapacity) * 1e9 / dt;
			bottomRelease = resRelease - spill;
			resStorage  = resCapacity; // This guarantees that the reservoir storage cannot exceed the reservoir capacity
		} else if (resStorage < 0.03 * resCapacity) { // ARIEL EDITS (fraction of rescapacity) from 0.1 to 0.03
			resRelease  = (prevResStorage - 0.03 * resCapacity) * 1e9 / dt + discharge;
			resStorage  = 0.03 * resCapacity;
			bottomRelease = resRelease;
			resExtRelease = resRelease + spill;
		}
		resStorageChg = resStorage - prevResStorage;
	}
	/// BALAZS TODO -> MODEL SHOULD OUTPUT Spill, BOTTOM RELEASE, AND TOTAL RELEASE. 
	MFVarSetFloat (_MDInResInitStorageID,         itemID, resInitStorage);
	MFVarSetFloat (_MDOutResStorageID,            itemID, resStorage); 
	MFVarSetFloat (_MDOutResStorageChgID,         itemID, resStorageChg); 
	MFVarSetFloat (_MDOutResInflowID,             itemID, resInflow);
	MFVarSetFloat (_MDOutResReleaseID,            itemID, resRelease);
	MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, resExtRelease);
	MFVarSetFloat (_MDOutResTargetReleaseID,      itemID, releaseTarget); // for Debuging only
	MFVarSetFloat (_MDOutResAttemptedReleaseID,   itemID, resRelease);    // for Debuging only
	MFVarSetFloat (_MDOutResBottomReleaseID,      itemID, bottomRelease);
	MFVarSetFloat (_MDOutResSpillID,              itemID, spill);
}

enum { MDhelp, MDwisser, MDsnl };

int MDReservoir_OperationDef () {
	int optID = MDwisser;
	const char *optStr, *options [ ] = { MFhelpStr, "Wisser", "SNL" , (char *) NULL };

	if (_MDOutResReleaseID != MFUnset) return (_MDOutResReleaseID);

	MFDefEntering ("Reservoirs");
	if ((optStr = MFOptionGet (MDVarReservoir_Release)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
 	switch (optID) {
		default:
		case MDhelp: MFOptionMessage (MDVarReservoir_Release, optStr, options); return (CMfailed);
		case MDwisser:
			if (((_MDInRouting_DischargeID      = MDRouting_ChannelDischargeDef())   == CMfailed) ||
				((_MDInResUptakeID              = MDReservoir_UptakeDef ())          == CMfailed) ||
				((_MDInAux_MeanDischargeID      = MDAux_DischargeMeanDef ())         == CMfailed) ||
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID            = MFVarGetID (MDVarReservoir_Storage,            "km3",  MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID         = MFVarGetID (MDVarReservoir_StorageChange,      "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResInflowID             = MFVarGetID (MDVarReservoir_Inflow,             "m3/s", MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID            = MFVarGetID (MDVarReservoir_Release,            "m3/s", MFOutput, MFState, MFBoundary)) == CMfailed) ||
 			    ((_MDOutResExtractableReleaseID = MFVarGetID (MDVarReservoir_ExtractableRelease, "m3/s", MFRoute,  MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirWisser) == CMfailed)) return (CMfailed);
			break;
		case MDsnl:
			if (((_MDInRouting_DischargeID      = MDRouting_ChannelDischargeDef()) == CMfailed) ||
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",    MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResInitStorageID         = MFVarGetID (MDVarReservoir_InitStorage,        "km3",    MFInput,  MFState, MFInitial))  == CMfailed) ||
			    ((_MDInResNatFlowMeanMonthlyID  = MFVarGetID ("ReservoirNatInflowMeanMonthly",   "m3/s",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
			    ((_MDInResNatFlowMeanDailyID    = MFVarGetID ("ReservoirNatInflowMeanDaily",     "m3/s",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResStorageRatioID        = MFVarGetID ("ReservoirStorageRatio",           MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResStorageRatio25ID      = MFVarGetID ("ReservoirStorageRatio25",         MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResStorageRatio75ID      = MFVarGetID ("ReservoirStorageRatio75",         MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResDemandFactorID        = MFVarGetID ("ReservoirDemandFactor",           MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResIncMultID             = MFVarGetID ("ReservoirIncMult",                MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResIncrement1ID          = MFVarGetID ("ReservoirIncrement1",             MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResIncrement2ID          = MFVarGetID ("ReservoirIncrement2",             MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResIncrement3ID          = MFVarGetID ("ReservoirIncrement3",             MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResAlphaID               = MFVarGetID ("ReservoirAlpha",                  MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResReleaseAdjID          = MFVarGetID ("Reservoir_ReleaseAdj",            MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResTargetReleaseID      = MFVarGetID ("Reservoir_TargetRelease",         "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResAttemptedReleaseID   = MFVarGetID ("Reservoir_AttemptedRelease",      "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResBottomReleaseID      = MFVarGetID ("Reservoir_BottomRelease",         "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResSpillID              = MFVarGetID ("Reservoir_Spill",                 "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID            = MFVarGetID (MDVarReservoir_Storage,            "km3",    MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID         = MFVarGetID (MDVarReservoir_StorageChange,      "km3",    MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResInflowID             = MFVarGetID (MDVarReservoir_Inflow,             "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
 			    ((_MDOutResExtractableReleaseID = MFVarGetID (MDVarReservoir_ExtractableRelease, "m3/s",   MFRoute,  MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID            = MFVarGetID (MDVarReservoir_Release,            "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirSNL) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Reservoirs");
	return (_MDOutResReleaseID); 
}
