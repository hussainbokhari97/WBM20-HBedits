/******************************************************************************
GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDWTemp_RiverBottom.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <string.h>
#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInRouting_DischargeID  = MFUnset;
static int _MDInReservoir_StorageID  = MFUnset;
static int _MDInWTemp_RiverTopID     = MFUnset;
// Output
static int _MDOutWTemp_RiverBottomID = MFUnset;

#define MinTemp 1.0

static void _MDWTempRiverBottom (int itemID) {
// Input
    float discharge       = MFVarGetFloat (_MDInRouting_DischargeID, itemID, 0.0); // Discharge in m3/s 
    float storage         = _MDInReservoir_StorageID != MFUnset ?  MFVarGetFloat (_MDInReservoir_StorageID, itemID, 0.0) : 0.0;
   	float riverTempTop    = MFVarGetFloat (_MDInWTemp_RiverTopID,    itemID, 0.0); // Runoff temperature degC
// Output
    float riverTempBottom; // River bottom temprature in degC
// Model
    float dt = MFModelGet_dt (); // Model time step in seconds

    if (riverTempTop > 4.0)
        riverTempBottom = storage > 0.0 ? (storage * 0.1 * 1e9 / dt * 4.0 + discharge * riverTempTop) / (0.1 * 1e9 / dt + discharge) : riverTempTop;
    else
        riverTempBottom = riverTempTop;
    MFVarSetFloat(_MDOutWTemp_RiverBottomID, itemID, riverTempBottom);
}

int MDWTemp_RiverBottomDef () {
	if (_MDOutWTemp_RiverBottomID != MFUnset) return (_MDOutWTemp_RiverBottomID);

	MFDefEntering ("River bottom temperature");
	if (((_MDInRouting_DischargeID  = MDRouting_DischargeDef ())          == CMfailed) ||
        ((_MDInReservoir_StorageID  = MDReservoir_StorageDef ())          == CMfailed) ||
        ((_MDInWTemp_RiverTopID     = MDWTemp_RiverTopDef ())             == CMfailed) ||
        ((_MDOutWTemp_RiverBottomID = MFVarGetID (MDVarWTemp_RiverBottom, "degC", MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempRiverBottom) == CMfailed)) return (CMfailed);
	   MFDefLeaving ("River bottom temperature");
	   return (_MDOutWTemp_RiverBottomID);
}