/******************************************************************************
 GHAAS Water Balance/Transport Model
 Global Hydrological Archive and Analysis System
 Copyright 1994-2022, UNH - ASRC/CUNY

 MDCommon_HumidityVaporPressure.c

 amiara@ccny.cuny.edu

 *******************************************************************************/

/********************************************************************************
 * Calculates Specific Humidity from pressure, air temperature and rh
 * ******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_HumiditySpecificID       = MFUnset;
static int _MDInCommon_AirPressureID            = MFUnset;
// Output
static int _MDOutCommon_HumidityVaporPressureID = MFUnset;

static void _MDCommon_HumidityVaporPressure (int itemID) {
    float specificHumidity; // Specific humidity kg/kg
    float airPressure;      // Air pressure in kPa
    float vaporPressure;    // Vapor pressure in kPa

    specificHumidity = MFVarGetFloat (_MDInCommon_HumiditySpecificID, itemID, 0.0);
    airPressure      = MFVarGetFloat (_MDInCommon_AirPressureID,      itemID, 0.0); // air pressure (kPa)

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
        case MFinput: _MDOutCommon_HumidityVaporPressureID = MFVarGetID (MDVarCommon_HumidityVaporPressure, "Pa", MFInput, MFState, MFBoundary); break;
        case MFcalculate:
            if (((_MDInCommon_HumiditySpecificID       = MDCommon_HumiditySpecificDef ()) == CMfailed) ||
                ((_MDInCommon_AirPressureID            = MFVarGetID (MDVarCommon_AirPressure,           "kPa", MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutCommon_HumidityVaporPressureID = MFVarGetID (MDVarCommon_HumidityVaporPressure, "Pa",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((MFModelAddFunction (_MDCommon_HumidityVaporPressure) == CMfailed))) return (CMfailed);
            break;
    }
    MFDefLeaving ("VaporPressure");
    return (_MDOutCommon_HumidityVaporPressureID);
}
