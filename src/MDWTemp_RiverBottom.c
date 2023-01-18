/******************************************************************************
GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

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

int MDWTemp_RiverBottomDef () {
	if (_MDOutWTemp_RiverBottomID != MFUnset) return (_MDOutWTemp_RiverBottomID);

	MFDefEntering ("River bottom temperature");
	if (((_MDInRouting_DischargeID  = MDRouting_DischargeDef ()) == CMfailed) ||
        ((_MDInWTemp_RiverTopID     = MDWTemp_RiverTopDef ())    == CMfailed) ||
        ((_MDInReservoir_StorageID  = MDReservoir_StorageDef ()) == CMfailed) ||
        ((_MDOutWTemp_RiverBottomID = _MDInReservoir_StorageID != MFUnset ? MDWTemp_ReservoirBottomDef () : _MDInWTemp_RiverTopID) == CMfailed)) return (CMfailed);
	MFDefLeaving ("River bottom temperature");
	return (_MDOutWTemp_RiverBottomID);
}
