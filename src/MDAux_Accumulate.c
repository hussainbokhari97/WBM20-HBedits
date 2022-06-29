/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDAux_Accumulate.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

//Input;
static int _MDInCommon_PrecipID     = MFUnset;
//Output
static int _MDOutAux_AccPrecipID = MFUnset;

static void _MDAux_AccumPrecip (int itemID) {
	float accum, value;
	
	value = MFVarGetFloat (_MDInCommon_PrecipID,  itemID, 0.0) * MFModelGetArea (itemID) / (MFModelGet_dt () * 1000.0); // Converting to m3/s
	accum = MFVarGetFloat (_MDOutAux_AccPrecipID, itemID, 0.0);
	MFVarSetFloat(_MDOutAux_AccPrecipID, itemID,accum + value);
}

int MDAux_AccumPrecipDef() {

	if (_MDOutAux_AccPrecipID != MFUnset) return (_MDOutAux_AccPrecipID);

	MFDefEntering ("Accumulate Precipitation");
	if (((_MDInCommon_PrecipID  = MDCommon_PrecipitationDef()) == CMfailed) ||
        ((_MDOutAux_AccPrecipID = MFVarGetID (MDVarAux_AccPrecipitation, "m3/s", MFRoute, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction(_MDAux_AccumPrecip) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Accumulate Precipitation");
	return (_MDOutAux_AccPrecipID);	
}

//Input;
static int _MDInAux_EvapID      = MFUnset;
//Output
static int _MDOutAux_AccEvapID  = MFUnset;

static void _MDAux_AccumEvap (int itemID) {
	float accum, value;
	
	value = MFVarGetFloat (_MDInAux_EvapID,     itemID, 0.0) * MFModelGetArea (itemID) / (MFModelGet_dt () * 1000.0); // Converting to m3/s
	accum = MFVarGetFloat (_MDOutAux_AccEvapID, itemID, 0.0);
	MFVarSetFloat(_MDOutAux_AccEvapID, itemID, accum + value);
}

int MDAux_AccumEvapDef() {

	if (_MDOutAux_AccEvapID != MFUnset) return (_MDOutAux_AccEvapID);

	MFDefEntering ("Accumulate Evapotranspiration");
	if (((_MDInAux_EvapID     = MDCore_EvapotranspirationDef()) == CMfailed) ||
        ((_MDOutAux_AccEvapID = MFVarGetID (MDVarAux_AccEvapotranspiration, "m3/s", MFRoute, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction(_MDAux_AccumEvap) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Accumulate Evapotranspiration");
	return (_MDOutAux_AccEvapID);
}

//Input;
static int _MDInAux_SnowPackChgID     = MFUnset;
//Output
static int _MDOutAux_AccSnowPackChgID = MFUnset;

static void _MDAux_AccumSnowPackChage (int itemID) {
	float accum, value;
	
	value = MFVarGetFloat (_MDInAux_SnowPackChgID,     itemID, 0.0) * MFModelGetArea (itemID) / (MFModelGet_dt () * 1000.0); // Converting to m3/s
	accum = MFVarGetFloat (_MDOutAux_AccSnowPackChgID, itemID, 0.0);
	MFVarSetFloat(_MDOutAux_AccSnowPackChgID, itemID, accum + value);
}

int MDAux_AccumSnowPackChgDef() {

	if (_MDOutAux_AccEvapID != MFUnset) return (_MDOutAux_AccEvapID);

	MFDefEntering ("Accumulate Snowpack Change");
	if (((_MDInAux_SnowPackChgID     = MDCore_SnowPackChgDef ()) == CMfailed) ||
        ((_MDOutAux_AccSnowPackChgID = MFVarGetID (MDVarAux_AccSnowPackChange, "m3/s", MFRoute, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction(_MDAux_AccumSnowPackChage) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Accumulate Snowpack Change");
	return (_MDOutAux_AccSnowPackChgID);
}

//Input;
static int _MDInAux_SMoistChgID     = MFUnset;
//Output
static int _MDOutAux_AccSMoistChgID = MFUnset;

static void _MDAux_AccumSMoistChg (int itemID) {
	float accum, value;
	
	value = MFVarGetFloat (_MDInAux_SMoistChgID,     itemID, 0.0) * MFModelGetArea (itemID) / (MFModelGet_dt () * 1000.0); // Converting to m3/s
	accum = MFVarGetFloat (_MDOutAux_AccSMoistChgID, itemID, 0.0);
	MFVarSetFloat(_MDOutAux_AccSMoistChgID, itemID, accum + value);
}

int MDAux_AccumSMoistChgDef() {

	if (_MDOutAux_AccSMoistChgID != MFUnset) return (_MDOutAux_AccSMoistChgID);

	MFDefEntering ("Accumulate Soil Moisture Change");
	if (((_MDInAux_SMoistChgID     = MDCore_SoilMoistChgDef()) == CMfailed) ||
        ((_MDOutAux_AccSMoistChgID = MFVarGetID (MDVarAux_AccSoilMoistChange, "m3/s", MFRoute, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction(_MDAux_AccumSMoistChg) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Accumulate Soil Moisture Change");
	return (_MDOutAux_AccSMoistChgID);
}

//Input;
static int _MDInAux_GrdWatChgID     = MFUnset;
//Output
static int _MDOutAux_AccGrdWatChgID = MFUnset;

static void _MDAux_AccumGrdWatChg (int itemID) {
	float accum, value;

	value = MFVarGetFloat (_MDInAux_GrdWatChgID,     itemID, 0.0) * MFModelGetArea (itemID) / (MFModelGet_dt () * 1000.0); // Converting to m3/s
	accum = MFVarGetFloat (_MDOutAux_AccGrdWatChgID, itemID, 0.0);
	MFVarSetFloat(_MDOutAux_AccGrdWatChgID, itemID, accum + value);
}

int MDAux_AccumGrdWatChgDef() {

	if (_MDOutAux_AccGrdWatChgID != MFUnset) return (_MDOutAux_AccGrdWatChgID);

	MFDefEntering ("Accumulate Groundwater Change");
	if (((_MDInAux_GrdWatChgID     = MFVarGetID (MDVarCore_GroundWaterChange,   "mm",   MFInput, MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutAux_AccGrdWatChgID = MFVarGetID (MDVarAux_AccGroundWaterChange, "m3/s", MFRoute, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction(_MDAux_AccumGrdWatChg) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Accumulate Groundwater Change");
	return (_MDOutAux_AccGrdWatChgID);	
}

//Input;
static int _MDInCore_RunoffVolumeID  = MFUnset;
//Output
static int _MDOutAux_AccCore_RunoffID    = MFUnset;

static void _MDAux_AccumRunoff (int itemID) {
	float accum, value;

	value = MFVarGetFloat (_MDInCore_RunoffVolumeID,   itemID, 0.0);
	accum = MFVarGetFloat (_MDOutAux_AccCore_RunoffID, itemID, 0.0);

	MFVarSetFloat(_MDOutAux_AccCore_RunoffID, itemID, accum + value);
}

int MDAux_AccumRunoffDef() {

	if (_MDOutAux_AccCore_RunoffID != MFUnset) return (_MDOutAux_AccCore_RunoffID);

	MFDefEntering ("Accumulate Runoff");
	if (((_MDInCore_RunoffVolumeID   = MDCore_RunoffVolumeDef()) == CMfailed) ||
        ((_MDOutAux_AccCore_RunoffID = MFVarGetID (MDVarAux_AccRunoff, "m3/s", MFRoute, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction(_MDAux_AccumRunoff) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Accumulate Runoff");
	return (_MDOutAux_AccCore_RunoffID);	
}

//Input;
static int _MDInCore_RiverStorageChgID    = MFUnset;
//Output
static int _MDOutAux_AccRiverStorageChgID = MFUnset;

static void _MDAux_AccumRiverStorageChg (int itemID) {
	float accum, value;

	value = MFVarGetFloat (_MDInCore_RiverStorageChgID,    itemID, 0.0);
	accum = MFVarGetFloat (_MDOutAux_AccRiverStorageChgID, itemID, 0.0);

	MFVarSetFloat(_MDOutAux_AccRiverStorageChgID, itemID, accum + value);
}

int MDAux_AccumRiverStorageChg () {

	if (_MDInCore_RiverStorageChgID != MFUnset) return (_MDInCore_RiverStorageChgID);

	MFDefEntering ("Accumulate River Storage Change");
	if (((_MDInCore_RiverStorageChgID    = MDRouting_ChannelStorageDef()) == CMfailed) ||
        ((_MDOutAux_AccRiverStorageChgID = MFVarGetID (MDVarRouting_RiverStorageChg, "m3", MFRoute, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction(_MDAux_AccumRiverStorageChg) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Accumulate River Storage Change");
	return (_MDOutAux_AccRiverStorageChgID);	
}