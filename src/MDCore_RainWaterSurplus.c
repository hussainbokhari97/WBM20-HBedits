/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDCore_RainWaterSurplus.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInSnowPackChgID         = MFUnset;
static int _MDInRainSMoistChgID       = MFUnset;
static int _MDInRainEvapoTransID      = MFUnset;
static int _MDInCommon_PrecipID       = MFUnset;
static int _MDInIrrigation_AreaFracID = MFUnset;
// Output
static int _MDOutRainWaterSurplusID = MFUnset;

static void _MDRainWaterSurplus (int itemID) {
// Input
	float irrAreaFrac = _MDInIrrigation_AreaFracID != MFUnset ? MFVarGetFloat (_MDInIrrigation_AreaFracID, itemID, 0.0) : 0.0;
	float sPackChg    = MFVarGetFloat (_MDInSnowPackChgID,    itemID, 0.0); // No irrigaiton when snow is on the ground
	float sMoistChg   = MFVarGetFloat (_MDInRainSMoistChgID,  itemID, 0.0) * (1.0 - irrAreaFrac);
	float evapoTrans  = MFVarGetFloat (_MDInRainEvapoTransID, itemID, 0.0) * (1.0 - irrAreaFrac); 
	float precip      = MFVarGetFloat (_MDInCommon_PrecipID,  itemID, 0.0) * (1.0 - irrAreaFrac);
// Output
	float surplus;
 
	surplus = precip - sPackChg - evapoTrans - sMoistChg;
	MFVarSetFloat (_MDOutRainWaterSurplusID, itemID, surplus);
}

int MDCore_RainWaterSurplusDef () {
    int ret;

	if (_MDOutRainWaterSurplusID != MFUnset) return (_MDOutRainWaterSurplusID);

	MFDefEntering ("Rainfed Water Surplus");
    if (((ret = MDIrrigation_GrossDemandDef ()) == CMfailed) ||
        ((ret != MFUnset) && ((_MDInIrrigation_AreaFracID = MDIrrigation_IrrAreaDef ()) == CMfailed)))
        return (CMfailed);

    if (((_MDInRainSMoistChgID     = MDCore_RainSMoistChgDef ())   == CMfailed) ||
        ((_MDInCommon_PrecipID     = MDCommon_PrecipitationDef ()) == CMfailed) ||
        ((_MDInSnowPackChgID       = MDCore_SnowPackChgDef ())     == CMfailed) ||
        ((_MDInRainEvapoTransID    = MFVarGetID (MDVarCore_RainEvapotranspiration, "mm", MFInput,  MFFlux, MFBoundary)) == CMfailed) ||
        ((_MDOutRainWaterSurplusID = MFVarGetID (MDVarCore_RainWaterSurplus,       "mm", MFOutput, MFFlux, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDRainWaterSurplus) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Rainfed Water Surplus");
	return (_MDOutRainWaterSurplusID);
}
