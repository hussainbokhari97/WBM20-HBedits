/******************************************************************************
GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDWTemp_ReservoirBottom.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <string.h>
#include <math.h>
#include <MF.h>
#include <MD.h>
#include <res_strat.h>

// Input
static int _MDInRouting_DischargeID  = MFUnset;
static int _MDInReservoir_StorageID  = MFUnset;
static int _MDInWTemp_RiverTopID     = MFUnset;
static int _MDInStrat_GMjID          = MFUnset;
static int _MDInStrat_DepthID        = MFUnset;
static int _MDInStrat_HeightID       = MFUnset;
static int _MDInStrat_LengthID       = MFUnset;
static int _MDInStrat_WidthID        = MFUnset;
static int _MDInStrat_VolumeErrID    = MFUnset;
static int _MDInStrat_AreaErrID      = MFUnset;
static int _MDInStrat_VolumeCoeffID  = MFUnset;
static int _MDInStrat_AreaCoeffID    = MFUnset;
static int _MDInStrat_VolumeDiffID   = MFUnset;
static int _MDInStrat_AreaDiffID     = MFUnset;
static int _MDInStrat_NDepthID       = MFUnset;
// State
#define MDStrat_MaxLayer 30
static int _MDStateStrat_dZ  [MDStrat_MaxLayer] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset }; 
static int _MDStateStrat_tZ  [MDStrat_MaxLayer] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset };
static int _MDStateStrat_rho [MDStrat_MaxLayer] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset };
static int _MDStateStrat_aD  [MDStrat_MaxLayer] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset };
static int _MDStateStrat_mZn [MDStrat_MaxLayer] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset };
static int _MDStateStrat_dV  [MDStrat_MaxLayer] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset };
static int _MDStateStrat_vZt [MDStrat_MaxLayer] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset };
// Output
static int _MDOutWTemp_ReservoirBottomID = MFUnset;

#define MinTemp 1.0

static void _MDWTempReservoirBottom (int itemID) {
// Input
    float discharge       = MFVarGetFloat (_MDInRouting_DischargeID, itemID, 0.0); // Discharge in m3/s 
    float storage         = _MDInReservoir_StorageID != MFUnset ?  MFVarGetFloat (_MDInReservoir_StorageID, itemID, 0.0) : 0.0;
   	float riverTempTop    = MFVarGetFloat (_MDInWTemp_RiverTopID,    itemID, 0.0); // Runoff temperature degC
    struct reservoir_geometry resgeo;
// Output
    float riverTempBottom; // River bottom temprature in degC
// Model
    float dt = MFModelGet_dt (); // Model time step in seconds

    resgeo.gm_j    = MFVarGetInt   (_MDInStrat_GMjID,         itemID, 0);
    resgeo.depth   = MFVarGetFloat (_MDInStrat_DepthID,       itemID, 0.0);
    resgeo.d_ht    = MFVarGetFloat (_MDInStrat_HeightID,      itemID, 0.0);
    resgeo.M_L     = MFVarGetFloat (_MDInStrat_LengthID,      itemID, 0.0);
    resgeo.M_W     = MFVarGetFloat (_MDInStrat_WidthID,       itemID, 0.0);
    resgeo.V_err   = MFVarGetFloat (_MDInStrat_VolumeErrID,   itemID, 0.0);
    resgeo.Ar_err  = MFVarGetFloat (_MDInStrat_AreaErrID,     itemID, 0.0);
    resgeo.C_v     = MFVarGetFloat (_MDInStrat_VolumeCoeffID, itemID, 0.0);
    resgeo.C_a     = MFVarGetFloat (_MDInStrat_AreaCoeffID,   itemID, 0.0);
    resgeo.V_df    = MFVarGetFloat (_MDInStrat_VolumeDiffID,  itemID, 0.0);
    resgeo.A_df    = MFVarGetFloat (_MDInStrat_VolumeDiffID,  itemID, 0.0);
    resgeo.n_depth = MFVarGetInt   (_MDInStrat_NDepthID,      itemID, 0);
    riverTempBottom = riverTempTop;
    MFVarSetFloat(_MDOutWTemp_ReservoirBottomID, itemID, riverTempBottom);
}

int MDWTemp_ReservoirBottomDef () {
    int i;
    char stateName [64];
	if (_MDOutWTemp_ReservoirBottomID != MFUnset) return (_MDOutWTemp_ReservoirBottomID);

	MFDefEntering ("Reservoir bottom temperature");
	if (((_MDInRouting_DischargeID      = MDRouting_DischargeDef ()) == CMfailed) ||
        ((_MDInReservoir_StorageID      = MDReservoir_StorageDef ()) == CMfailed) ||
        ((_MDInWTemp_RiverTopID         = MDWTemp_RiverTopDef ())    == CMfailed) ||
        ((_MDInStrat_GMjID              = MFVarGetID ("ReservoirGeometry",    MFNoUnit, MFInt,    MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_DepthID            = MFVarGetID ("ReservoirMeanDepth",   "m",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_HeightID           = MFVarGetID ("ReservoirDamHeight",   "m",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_LengthID           = MFVarGetID ("ReservoirLength",      "km",     MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_WidthID            = MFVarGetID ("ReservoirWidth",       "km",     MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_VolumeErrID        = MFVarGetID ("ReservoirVolumeError", "%",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_AreaErrID          = MFVarGetID ("ReservoirAreaError",   "%",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_VolumeCoeffID      = MFVarGetID ("ReservoirVolumeCoeff", MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_AreaCoeffID        = MFVarGetID ("ReservoirAreaCoeff",   MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_VolumeDiffID       = MFVarGetID ("ReservoirVolumeDiff",  "mcm",    MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_AreaDiffID         = MFVarGetID ("ReservoirAreaDiff",    "km2",    MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_NDepthID           = MFVarGetID ("NDepth",               MFNoUnit, MFInt,    MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutWTemp_ReservoirBottomID = MFVarGetID (MDVarWTemp_RiverTop,    "degC",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempReservoirBottom) == CMfailed)) return (CMfailed);
    for (i = 0; i < MDStrat_MaxLayer; ++i) {
        if ((snprintf (stateName,strlen(stateName),"dZ%02d",  i) == 0) || ((_MDStateStrat_dZ  [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            (snprintf (stateName,strlen(stateName),"tZ%02d",  i) == 0) || ((_MDStateStrat_tZ  [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            (snprintf (stateName,strlen(stateName),"rho%02d", i) == 0) || ((_MDStateStrat_rho [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            (snprintf (stateName,strlen(stateName),"aD%02d",  i) == 0) || ((_MDStateStrat_aD  [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            (snprintf (stateName,strlen(stateName),"mZn%02d", i) == 0) || ((_MDStateStrat_mZn [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            (snprintf (stateName,strlen(stateName),"dV%02d",  i) == 0) || ((_MDStateStrat_dV  [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            (snprintf (stateName,strlen(stateName),"vZt%02d", i) == 0) || ((_MDStateStrat_vZt [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed))
            return (CMfailed);
    }
	MFDefLeaving ("Reservoir bottom temperature");
	return (_MDOutWTemp_ReservoirBottomID);
}