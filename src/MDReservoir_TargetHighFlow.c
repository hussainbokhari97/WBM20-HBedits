/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDReservoir_TargetHighFlow.c

bfekete@gc.cuny.edu

******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInRouting_DischargeID      = MFUnset;
static int _MDInAux_MeanDischargeID      = MFUnset;
static int _MDInAux_MaxDischargeID       = MFUnset;
static int _MDInResCapacityID            = MFUnset;

// Output
static int _MDOutResSurplusID            = MFUnset;
static int _MDOutResAccumSurplusID       = MFUnset;
static int _MDOutResMaxAccumSurplusID    = MFUnset;
static int _MDOutResTargetHighFlowID     = MFUnset;

static void _MDReservoirTargetHighFlow (int itemID) {
// Input
	float discharge;       // Current discharge [m3/s]
	float meanDischarge;   // Long-term mean annual discharge [m3/s]
	float maxDischarge;    // Long-term minimum discharge [m3/s]
	float resCapacity;     // Reservoir capacity [km3]
// Output
	float surplus;         // Inflow surplus (above mean annual discharge [m3/s]
	float accumSurplus;    // Accumulated surplus during the season [m3/s]
	float maxAccumSurplus; // Maximum accumulated surplus [m3/s]
// Local
	float targetHighFlow;  // Targeted minimum flow [m3/s]
	float optResCapacity;  // Reservoir capacity required for fully eliminating low flows;
	float dt;              // Time step length [s]

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) <= 0.0) { 
		MFVarSetFloat (_MDOutResSurplusID,            itemID, 0.0);
		MFVarSetFloat (_MDOutResAccumSurplusID,       itemID, 0.0);
		MFVarSetFloat (_MDOutResMaxAccumSurplusID,    itemID, 0.0);
		MFVarSetFloat (_MDOutResTargetHighFlowID,     itemID, 0.0);
		return;
	}
	             dt = MFModelGet_dt ();
	 discharge      = MFVarGetFloat (_MDInRouting_DischargeID,   itemID, 0.0);
	 meanDischarge  = MFVarGetFloat (_MDInAux_MeanDischargeID,   itemID, discharge);
	  maxDischarge  = MFVarGetFloat (_MDInAux_MaxDischargeID,    itemID, discharge);
	   accumSurplus = MFVarGetFloat (_MDOutResAccumSurplusID,    itemID, 0.0);
	maxAccumSurplus = MFVarGetFloat (_MDOutResMaxAccumSurplusID, itemID, discharge);

	       surplus  = discharge - meanDischarge;
	   accumSurplus = accumSurplus + surplus > 0.0 ? accumSurplus + surplus : 0.0;
	maxAccumSurplus = accumSurplus > maxAccumSurplus ? accumSurplus : maxAccumSurplus;
	optResCapacity  = maxAccumSurplus * dt / 1e9;
	 targetHighFlow = maxDischarge - (maxDischarge - meanDischarge) * resCapacity / (optResCapacity > resCapacity ? optResCapacity : resCapacity);

	MFVarSetFloat (_MDOutResSurplusID,            itemID, surplus);
	MFVarSetFloat (_MDOutResAccumSurplusID,       itemID, accumSurplus);
	MFVarSetFloat (_MDOutResMaxAccumSurplusID,    itemID, maxAccumSurplus);
	MFVarSetFloat (_MDOutResTargetHighFlowID,     itemID, targetHighFlow);
}

int MDReservoir_TargetHighFlowDef () {
	int optID = MFnone;
	const char *optStr;

	if (_MDOutResTargetHighFlowID != MFUnset) return (_MDOutResTargetHighFlowID);

	MFDefEntering ("Target high flow");
	if ((optStr = MFOptionGet (MDVarReservoir_TargetHighFlow)) != (char *) NULL) optID = CMoptLookup (MFcalcOptions, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage (MDVarReservoir_TargetHighFlow, optStr, MFcalcOptions); return (CMfailed);
		case MFhelp: MFOptionMessage (MDVarReservoir_TargetHighFlow, optStr, MFcalcOptions);
		case MFnone: break;
		case MFinput:
			if ((_MDOutResTargetHighFlowID      = MFVarGetID (MDVarReservoir_TargetHighFlow,     "m3/s", MFInput,  MFState, MFBoundary)) == CMfailed)
				return (CMfailed);
			break;
		case MFcalculate: break;
			if (((_MDInAux_MeanDischargeID      = MDAux_MeanDischargeDef       ()) == CMfailed) ||
			    ((_MDInAux_MaxDischargeID       = MDAux_MaximumDischargeDef    ()) == CMfailed) ||
                ((_MDInRouting_DischargeID      = MDRouting_DischargeUptakeDef ()) == CMfailed) ||
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResSurplusID            = MFVarGetID (MDVarReservoir_Deficit,            "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResAccumSurplusID       = MFVarGetID (MDVarReservoir_AccumSurplus,       "m3/s", MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResMaxAccumSurplusID    = MFVarGetID (MDVarReservoir_MaxAccumSurplus,    "m3/s", MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResTargetHighFlowID     = MFVarGetID (MDVarReservoir_TargetLowFlow,      "m3/s", MFInput,  MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirTargetHighFlow) == CMfailed)) return (CMfailed);
	}
	MFDefLeaving ("Target high flow");
	return (_MDOutResTargetHighFlowID); 
}
