/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

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
static int _MDOutResStorageInitialID     = MFUnset;
static int _MDOutResInflowID             = MFUnset;
static int _MDOutResReleaseID            = MFUnset;
static int _MDOutResReleaseExtractableID = MFUnset;
static int _MDOutResReleaseBottomID      = MFUnset;
static int _MDOutResReleaseSpillwayID    = MFUnset;
static int _MDOutResReleaseTargetID      = MFUnset;

static void _MDReservoirWisser (int itemID) {
// Input
	float discharge;             // Current discharge [m3/s]
	float meanDischarge;         // Long-term mean annual discharge [m3/s]
	float resCapacity;           // Reservoir capacity [km3]
	float resUptake;             // Water uptake from reservoir [m3/s]
// Output
	float resStorage;            // Reservoir storage [km3]
	float resStorageChg;         // Reservoir storage change [km3/dt]
	float resInflow;             // Reservoir release [m3/s] 
	float resReleaseBottom;      // Reservoir release at the bottom [m3/s]
	float resReleaseExtract;     // Reservoir extractable release [m3/s]
	float resReleaseSpillway;    // Reservoir release via spillway [m3/s]
// local
	float prevResStorage;        // Reservoir storage from the previous time step [km3]
	float dt = MFModelGet_dt (); // Time step length [s]
	//float current_month = MFDateGetCurrentMonth (); Current Calendar Month
// Parameters
	float drySeasonPct = 0.60;   // RJS 071511
	float wetSeasonPct = 0.16;   // RJS 071511

	discharge = resInflow = MFVarGetFloat (_MDInRouting_DischargeID,      itemID, 0.0);
	resReleaseExtract     = MFVarGetFloat (_MDOutResReleaseExtractableID, itemID, 0.0);

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) > 0.0001) { // TODO Arbitrary limit      
		 meanDischarge = MFVarGetFloat (_MDInAux_MeanDischargeID,      itemID, discharge);
		prevResStorage = MFVarGetFloat(_MDOutResStorageID, itemID, 0.0);
		     resUptake = _MDInResUptakeID != MFUnset ? MFVarGetFloat (_MDInResUptakeID,itemID, 0.0) : 0.0; 

		if (prevResStorage * 1e9 / dt < resUptake) {
			resUptake = prevResStorage * 1e9 / dt;
			prevResStorage = 0.0;
		}
		resReleaseSpillway = 0.0;
		resReleaseBottom = resReleaseBottom = discharge > meanDischarge ? wetSeasonPct * discharge : drySeasonPct * discharge  + (meanDischarge - discharge);

 		resStorage = prevResStorage + (discharge - resUptake - resReleaseBottom) * dt / 1e9;
		if (resStorage > resCapacity) {
			resReleaseSpillway = (resStorage - resCapacity) * 1e9 / dt;
			resStorage         = resCapacity;
		}
		else if (resStorage < 0.0) {
			resReleaseBottom  = prevResStorage * 1e9 / dt + discharge;
			resStorage  = 0.0;
		}
		resStorageChg  = resStorage - prevResStorage;
		resReleaseExtract = resReleaseBottom + resReleaseSpillway > discharge ? resReleaseBottom + resReleaseSpillway - discharge : 0.0;
	} else {
		resStorage = resStorageChg = 0.0;
		resReleaseBottom   = discharge;
		resReleaseSpillway = 0.0;
	}
	MFVarSetFloat (_MDOutResStorageID,            itemID, resStorage);
	MFVarSetFloat (_MDOutResStorageChgID,         itemID, resStorageChg);
	MFVarSetFloat (_MDOutResInflowID,             itemID, resInflow);
	MFVarSetFloat (_MDOutResReleaseID,            itemID, resReleaseBottom + resReleaseSpillway);
	MFVarSetFloat (_MDOutResReleaseExtractableID, itemID, resReleaseExtract);
	MFVarSetFloat (_MDOutResReleaseBottomID,      itemID, resReleaseBottom);
	MFVarSetFloat (_MDOutResReleaseSpillwayID,    itemID, resReleaseSpillway);
}

static void _MDReservoirSNL (int itemID) {
	// Input
	float discharge;                // Current discharge [m3/s] -- this is the inflow to the dam
	float natFlowMeanMonthly;       // Naturalized long-term mean monthly inflow [m3/s]
	float natFlowMeanDaily;         // Naturalized long-term mean daily inflow [m3/s]
	float resCapacity;              // Reservoir capacity [km3]
	float prevResStorage;           // Reference storage dictatink actual release ratio [km3]
	// Output
	float resStorage;               // Reservoir storage [km3]
	float resStorageChg;            // Reservoir storage change [km3/dt]
	float resReleaseTarget;         // Target reservoir release [m3/s]
	float resReleaseSpillway;       // gets calculated further down, otherwise 0
	float resReleaseExtract;        // Reservoir extractable release [m3/s]
	float resReleaseBottom;         // Reservoir release [m3/s] 
	// Local
	float resInflow;                // Reservoir inflow [m3/s] 
	float waterDemandMeanDaily;
	float krls;                     // release ratio
	float initial_krls;  
	float waterDemandMeanMonthly;

	resReleaseBottom   =
	resInflow          =
	discharge          = MFVarGetFloat (_MDInRouting_DischargeID,      itemID, 0.0);
	resReleaseExtract  = MFVarGetFloat (_MDOutResReleaseExtractableID, itemID, 0.0);
	resCapacity        = MFVarGetFloat (_MDInResCapacityID,            itemID, 0.0);
	if (resCapacity > 0.0001) { // TODO Arbitrary limit!!!!
		// Inputs
		float demandFactor;         // monthly constant per dam
		float incMult;              // monthly constant per dam 
		float increment1;           // monthly constant per dam 
		float increment2;           // monthly constant per dam 
		float increment3;           // monthly constant per dam 
		float alpha;                // monthly constant per dam 
		float releaseAdj;           // monthly constant per dam 
		float storageRatio;         // ratio of the normal vs maximum storage (from NID)
		float resCapacity25;
		float resCapacity75;
		// Local
		float increment;
		float deadStorage = 0.03 * resCapacity;
		float dt = MFModelGet_dt ();  // Time step length [s]
		float current_month = MFDateGetCurrentMonth (); // Current Calendar Month
			prevResStorage = MFVarGetFloat (_MDOutResStorageID,           itemID, 0.0);
		natFlowMeanMonthly = MFVarGetFloat (_MDInResNatFlowMeanMonthlyID, itemID, 0.0);
		  natFlowMeanDaily = MFVarGetFloat (_MDInResNatFlowMeanDailyID,   itemID, 0.0);
 	          storageRatio = MFVarGetFloat (_MDInResStorageRatioID,       itemID, 0.0);
    	     resCapacity25 = MFVarGetFloat (_MDInResStorageRatio25ID,     itemID, 0.0) * resCapacity;
    	     resCapacity75 = MFVarGetFloat (_MDInResStorageRatio75ID,     itemID, 0.0) * resCapacity;
    	      demandFactor = MFVarGetFloat (_MDInResDemandFactorID,       itemID, 0.0);
    	           incMult = MFVarGetFloat (_MDInResIncMultID,            itemID, 0.0);
    	        increment1 = MFVarGetFloat (_MDInResIncrement1ID,         itemID, 0.0);
    	        increment2 = MFVarGetFloat (_MDInResIncrement2ID,         itemID, 0.0);
    	        increment3 = MFVarGetFloat (_MDInResIncrement3ID,         itemID, 0.0);
    	             alpha = MFVarGetFloat (_MDInResAlphaID,              itemID, 0.0);
			    releaseAdj = MFVarGetFloat (_MDInResReleaseAdjID,         itemID, 0.0);
		// ARIEL EDITED THIS TO INCULDE "{}" -- not sure it's needed, so please remove if not.
		if (prevResStorage <= 0.0) {prevResStorage = resCapacity; } // This could only happen before the model updates the initial storage
        
	// MAIN RULES begin ---->
		waterDemandMeanDaily   = demandFactor * natFlowMeanDaily;
		waterDemandMeanMonthly = demandFactor * natFlowMeanMonthly;                               
		// rough interpretation/goal for a release target    
		resReleaseTarget = natFlowMeanDaily + waterDemandMeanDaily - waterDemandMeanMonthly; 
		// krls before adjustment
		initial_krls = (prevResStorage / (alpha * storageRatio * resCapacity));
		// condition when storage is above 75% of normal or max
		if      (prevResStorage >= resCapacity75) increment = increment1;
		else if (prevResStorage >= resCapacity25) increment = increment2;
		else increment = increment3;
		// condition based on inflow
		if (resInflow >= natFlowMeanMonthly) krls = (1 + incMult * increment) * initial_krls;
		else krls = (1 + increment) * initial_krls;
		
		// adjustment to consider inflow on given day (should help with extremes)
		resReleaseBottom = 0.5 * ((krls * resReleaseTarget) + (releaseAdj * resInflow));
		resStorage = prevResStorage + (discharge - resReleaseBottom) * dt / 1e9;
		
	// MAIN RULES finish

		// HB and AM created the following condition to catch low releases
		current_month = MFDateGetCurrentMonth (); // Current Calendar Month
		
		if (resReleaseBottom < (0.2 * resInflow)) {
			if ((resStorage >= (0.75 * resCapacity)) && (resInflow > natFlowMeanMonthly)) {
				if ((current_month >= 4) && (current_month <= 8)) {
					resReleaseBottom = 1.05 * resInflow;
					resStorage = prevResStorage + (discharge - resReleaseBottom) * dt / 1e9;
				} else {
					resReleaseBottom = 0.95 * resInflow;
					resStorage = prevResStorage + (discharge - resReleaseBottom) * dt / 1e9;
				}

			} else {
				resReleaseBottom = resReleaseBottom;
			}
		}

		// assume 10% envrionemntal flow minimum, and makes sure there is no negative flow (changed from 0.05 to 0.1).
		if (resReleaseBottom < 0.10 * resInflow) {
			resReleaseBottom = resInflow * 0.10;
			resStorage = prevResStorage + (discharge - resReleaseBottom) * dt / 1e9;
		}

		// HB and AM created two additional sets of if statements to catch high release peaks

		// catching high releases
		// if resReleaseBottom > 5 * natFlowMeanMonthly
		// then if resStorageChg is positive (storage - prevstorage) and resStorage >= (0.9 * resCapacity) and resInflow > 2 * natFlowMeanMonthly
		// continue or just resReleaseBottom = resReleaseBottom
		// else, if 4 <= month <= 8, resReleaseBottom = 1.1 * resInflow, else resReleaseBottom = 0.9 * resInflow, recalculate storage at the end
		
		if (resReleaseBottom > 5 * natFlowMeanMonthly) {
			if (((resStorage - prevResStorage) > 0) && (resStorage >= (0.9 * resCapacity)) && (resInflow > 2 * natFlowMeanMonthly)) {
				resReleaseBottom = resReleaseBottom;
			} else {
				if ((current_month >= 4) && (current_month <= 8)) {
					resReleaseBottom = 1.05 * resInflow;
					resStorage = prevResStorage + (discharge - resReleaseBottom) * dt / 1e9;
				} else {
					resReleaseBottom = 0.95 * resInflow;
					resStorage = prevResStorage + (discharge - resReleaseBottom) * dt / 1e9;
				}
			}
		}


		
		// HB and AM created an additional set of if statements to catch large storage dips
		// if resStorageChg < (-0.09 * capacity)
		// then resReleaseBottom = ((0.09 * rescapacity) * 1e9 / dt) + resInflow
		// resStorage either recalculate or set to = prevResStorage - (0.09 * resCapacity)
		if ((resStorage - prevResStorage) < (-0.09 * resCapacity)) {
			resReleaseBottom = ((0.09 * resCapacity) * 1e9 / dt) + resInflow;
			resStorage = prevResStorage - (0.09 * resCapacity);
		}
		
		
		if (resStorage > resCapacity) {              // The reservoir over flows FBM
			resReleaseSpillway = (resStorage - resCapacity) * 1e9 / dt;
			resStorage  = resCapacity;               // This guarantees that the reservoir storage cannot exceed the reservoir capacity
		} else {
			resReleaseSpillway = 0.0;
			if (resStorage < deadStorage) {          // Ther reservoir empties out FBM
				if (prevResStorage > deadStorage) {  // Normally, the storage should stop at dead storage FBM
					resReleaseBottom  = (prevResStorage - deadStorage) * 1e9 / dt + discharge;
					resStorage        = deadStorage;	
				} else {                             // The reservoir is bellow dead storage during spinup FBM
					if (discharge > (deadStorage - prevResStorage) * 1e9 / dt) { // The discharge is more than missing volume to reach dead storage FBM
						resReleaseBottom = discharge - (deadStorage - prevResStorage) * 1e9 / dt;
						resStorage       = deadStorage;
					} else {                         // The discharge is less than the missing volume to reach dead storage FBM
						resReleaseBottom = 0.0;
						resStorage       = prevResStorage + discharge * dt / 1e9;
					}
				}
			}
		}
		resStorageChg = resStorage - prevResStorage;
		resReleaseExtract = resReleaseBottom + resReleaseSpillway > discharge ? resReleaseBottom + resReleaseSpillway - discharge : 0.0;
	} else { // River flow
		prevResStorage = resStorage = resStorageChg = resReleaseTarget = resReleaseSpillway = 0.0;
	}

	MFVarSetFloat (_MDOutResStorageInitialID,     itemID, prevResStorage);
	MFVarSetFloat (_MDOutResStorageID,            itemID, resStorage); 
	MFVarSetFloat (_MDOutResStorageChgID,         itemID, resStorageChg); 
	MFVarSetFloat (_MDOutResInflowID,             itemID, resInflow);
	MFVarSetFloat (_MDOutResReleaseID,            itemID, resReleaseBottom + resReleaseSpillway);
	MFVarSetFloat (_MDOutResReleaseBottomID,      itemID, resReleaseBottom);
	MFVarSetFloat (_MDOutResReleaseExtractableID, itemID, resReleaseExtract);
	MFVarSetFloat (_MDOutResReleaseSpillwayID,    itemID, resReleaseSpillway);
	MFVarSetFloat (_MDOutResReleaseTargetID,      itemID, resReleaseTarget); // for Debuging only
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
 			    ((_MDOutResReleaseExtractableID = MFVarGetID (MDVarReservoir_ReleaseExtractable, "m3/s", MFRoute,  MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseBottomID      = MFVarGetID (MDVarReservoir_ReleaseBottom,      "m3/s", MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseSpillwayID    = MFVarGetID (MDVarReservoir_ReleaseSpillway,    "m3/s", MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirWisser) == CMfailed)) return (CMfailed);
			break;
		case MDsnl:
			if (((_MDInRouting_DischargeID      = MDRouting_ChannelDischargeDef()) == CMfailed) ||
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",    MFInput,  MFState, MFBoundary)) == CMfailed) ||
			    ((_MDInResNatFlowMeanMonthlyID  = MFVarGetID ("ReservoirNatFlowMeanMonthly",     "m3/s",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
			    ((_MDInResNatFlowMeanDailyID    = MFVarGetID ("ReservoirNatFlowMeanDaily",       "m3/s",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResAlphaID               = MFVarGetID ("ReservoirAlpha",                  MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResDemandFactorID        = MFVarGetID ("ReservoirDemandFactor",           MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResIncMultID             = MFVarGetID ("ReservoirIncMult",                MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResIncrement1ID          = MFVarGetID ("ReservoirIncrement1",             MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResIncrement2ID          = MFVarGetID ("ReservoirIncrement2",             MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResIncrement3ID          = MFVarGetID ("ReservoirIncrement3",             MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResReleaseAdjID          = MFVarGetID ("ReservoirReleaseAdj",             MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResStorageRatioID        = MFVarGetID ("ReservoirStorageRatio",           MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResStorageRatio25ID      = MFVarGetID ("ReservoirStorageRatio25",         MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            	((_MDInResStorageRatio75ID      = MFVarGetID ("ReservoirStorageRatio75",         MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseTargetID      = MFVarGetID ("ReservoirReleaseTarget",          "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID            = MFVarGetID (MDVarReservoir_Storage,            "km3",    MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID         = MFVarGetID (MDVarReservoir_StorageChange,      "km3",    MFOutput, MFState, MFBoundary)) == CMfailed) ||
            	((_MDOutResStorageInitialID     = MFVarGetID (MDVarReservoir_StorageInitial,     "km3",    MFOutput, MFState, MFInitial))  == CMfailed) ||
			    ((_MDOutResInflowID             = MFVarGetID (MDVarReservoir_Inflow,             "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseSpillwayID    = MFVarGetID (MDVarReservoir_ReleaseSpillway,    "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
 			    ((_MDOutResReleaseExtractableID = MFVarGetID (MDVarReservoir_ReleaseExtractable, "m3/s",   MFRoute,  MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseBottomID      = MFVarGetID (MDVarReservoir_ReleaseBottom,      "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID            = MFVarGetID (MDVarReservoir_Release,            "m3/s",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirSNL) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Reservoirs");
	return (_MDOutResReleaseID); 
}

int MDReservoir_InflowDef () {
	if ((_MDOutResInflowID == MFUnset) &&  (MDReservoir_OperationDef () == CMfailed)) return (CMfailed);
	return (_MDOutResInflowID);
}
