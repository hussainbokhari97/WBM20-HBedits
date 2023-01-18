/******************************************************************************
GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDWTemp_RiverTop.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <string.h>
#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInRouting_Discharge0ID = MFUnset;
static int _MDInCore_RunoffFlowID    = MFUnset;
static int _MDInWTemp_RunoffID       = MFUnset;
// Route
static int _MDInWTemp_HeatFluxID     = MFUnset;
// Output
static int _MDOutWTemp_RiverTopID    = MFUnset;

#define MinTemp 1.0

static void _MDWTempRiverTop (int itemID) {
// Input
    float discharge0      = MFVarGetFloat (_MDInRouting_Discharge0ID,     itemID, 0.0); // Outflowing discharge in m3/s 
    float runoffFlow      = MFVarGetFloat (_MDInCore_RunoffFlowID,        itemID, 0.0); // RO volume in m3/s
   	float runoffTemp      = MFVarGetFloat (_MDInWTemp_RunoffID,           itemID, 0.0); // Runoff temperature degC
// Routed
    float heatFlux        = MFVarGetFloat (_MDInWTemp_HeatFluxID,        itemID, 0.0); // Heat flux degC * m3/s
// Output
    float riverTempTop;   // River temprature in degC
// Model
    float dt              = MFModelGet_dt ();          // Model time step in seconds
    float channelLength   = MFModelGetLength (itemID); // Channel length in m
    float cellArea        = MFModelGetArea (itemID);   // Cell area in m2
// Local
    float flowThreshold = cellArea * 0.0001 / dt; // 0.1 mm/day over the the cell area

    if (flowThreshold < runoffFlow) flowThreshold = runoffFlow;
    if (discharge0 > flowThreshold) { 
        heatFlux   += runoffTemp * runoffFlow * dt;
        riverTempTop = heatFlux / (discharge0 * dt);
        if (riverTempTop > 50.0) {
            CMmsgPrint (CMmsgWarning, "Day: %3d Cell: %10ld River Temperature: %6.1f\n", MFDateGetDayOfYear (), itemID, riverTempTop);
            riverTempTop = runoffTemp;
        }
    } else riverTempTop = runoffTemp;
    MFVarSetFloat(_MDOutWTemp_RiverTopID, itemID, riverTempTop);
}

int MDWTemp_RiverTopDef () {

	if (_MDOutWTemp_RiverTopID != MFUnset) return (_MDOutWTemp_RiverTopID);

	MFDefEntering ("River top temperature");
	if (((_MDInCore_RunoffFlowID         = MDCore_RunoffFlowDef ())                    == CMfailed) ||
        ((_MDInWTemp_RunoffID            = MDWTemp_RunoffDef ())                       == CMfailed) ||
        ((_MDInRouting_Discharge0ID      = MFVarGetID (MDVarRouting_Discharge0, "m3/s",      MFInput,  MFState, MFInitial))  == CMfailed) ||
        ((_MDInWTemp_HeatFluxID          = MFVarGetID (MDVarWTemp_HeatFlux,     "degC*m3/s", MFRoute,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutWTemp_RiverTopID         = MFVarGetID (MDVarWTemp_RiverTop,     "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempRiverTop) == CMfailed)) return (CMfailed);
	MFDefLeaving ("River top temperature");
	return (_MDOutWTemp_RiverTopID);
}