/******************************************************************************
GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2023, UNH - ASRC/CUNY

MDCommon_HumidityVaporPressure.c

bfekete@gc.cuny.edu

********************************************************************************/

#include <MF.h>
#include <MD.h>
#include <math.h>

// Input
static int _MDInCommon_HumiditySpecificID       = MFUnset;
static int _MDInCommon_AirPressureID            = MFUnset;
// Output
static int _MDOutCommon_HumidityVaporPressureID = MFUnset;

static void _MDCommon_HumidityVaporPressure (int itemID) {
// Input
    float specificHumidity = MFVarGetFloat (_MDInCommon_HumiditySpecificID, itemID, 0.0); // Specific humidity kg/kg
    float airPressure      = MFVarGetFloat (_MDInCommon_AirPressureID,      itemID, 0.0); // Air pressure in Pa
// Output
    float vaporPressure;    // Vapor pressure in Pa

    vaporPressure = specificHumidity * airPressure / (0.622 + specificHumidity * 0.378);
    // 0.378 comes from if specific humidity is defined as density of water vapor over density of moist air
    MFVarSetFloat(_MDOutCommon_HumidityVaporPressureID, itemID, vaporPressure);
}

int MDCommon_HumidityVaporPressureDef () {
    int optID = MFinput;
    const char *optStr;
     
    if (_MDOutCommon_HumidityVaporPressureID != MFUnset) return (_MDOutCommon_HumidityVaporPressureID);    

    MFDefEntering ("VaporPressure");
    if ((optStr = MFOptionGet(MDVarCommon_HumidityVaporPressure)) != (char *) NULL) optID = CMoptLookup(MFsourceOptions, optStr, true);
    switch (optID) {
        default:
        case MFhelp: MFOptionMessage (MDVarCommon_HumidityVaporPressure, optStr, MFsourceOptions); return (CMfailed);
        case MFinput: _MDOutCommon_HumidityVaporPressureID = MFVarGetID (MDVarCommon_HumidityVaporPressure, "kPa", MFInput, MFState, MFBoundary); break;
        case MFcalculate:
            if (((_MDInCommon_HumiditySpecificID       = MDCommon_HumiditySpecificDef ()) == CMfailed) ||
                ((_MDInCommon_AirPressureID            = MFVarGetID (MDVarCommon_AirPressure,           "kPa", MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutCommon_HumidityVaporPressureID = MFVarGetID (MDVarCommon_HumidityVaporPressure, "kPa", MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((MFModelAddFunction (_MDCommon_HumidityVaporPressure) == CMfailed))) return (CMfailed);
            break;
    }
    MFDefLeaving ("VaporPressure");
    return (_MDOutCommon_HumidityVaporPressureID);
}
