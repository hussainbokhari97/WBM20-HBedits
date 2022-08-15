/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDCommon_HumiditySpecific.c

bfekete@gc.cuny.edu

********************************************************************************/

#include <MF.h>
#include <MD.h>
#include <math.h>

// Input
static int _MDInCommon_AirPressureID                 = MFUnset;
static int _MDInCommon_HumiditySaturatedVaporPressID = MFUnset;
static int _MDInCommon_HumidityRelativeID            = MFUnset;

// Output
static int _MDOutCommon_HumiditySpecificID = MFUnset;

static void _MDCommon_HumiditySpecific (int itemID) {
// Input
    float airPressure      = MFVarGetFloat (_MDInCommon_AirPressureID,                 itemID, 0.0); // Air pressure in kPa
    float saturatedVP      = MFVarGetFloat (_MDInCommon_HumiditySaturatedVaporPressID, itemID, 0.0); // Saturated vapor pressure in Pa
    float relativeHumidity = MFVarGetFloat (_MDInCommon_HumidityRelativeID,            itemID, 0.0); // Relative humidity in percent
// Output
    float specificHumidity; // Specific humidity in kg/kg

    specificHumidity  = 0.622 * relativeHumidity / 100.0 * saturatedVP / (airPressure - 0.378 * relativeHumidity / 100.0 * saturatedVP);
    // 0.378 comes from if specific humidity is defined as density of water vapor over density of moist air
    MFVarSetFloat(_MDOutCommon_HumiditySpecificID, itemID, specificHumidity);
}

int MDCommon_HumiditySpecificDef () {
    int optID = MFinput;
    const char *optStr;
    
    if (_MDOutCommon_HumiditySpecificID != MFUnset) return (_MDOutCommon_HumiditySpecificID);
    
    MFDefEntering ("SpecificHumidity");
    if ((optStr = MFOptionGet (MDVarCommon_HumiditySpecific)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
    switch (optID) {
        default:
        case MFhelp: MFOptionMessage (MDVarCommon_HumiditySpecific, optStr, MFsourceOptions); return (CMfailed);
        case MFinput: _MDOutCommon_HumiditySpecificID = MFVarGetID (MDVarCommon_HumiditySpecific, "kg/kg", MFInput, MFState, MFBoundary); break;
        case MFcalculate:
            if (((_MDInCommon_HumiditySaturatedVaporPressID  = MDCommon_HumiditySaturatedVaporPressureDef ()) == CMfailed) ||
                ((_MDInCommon_HumidityRelativeID             = MDCommon_HumidityRelativeDef())                == CMfailed) ||
                ((_MDInCommon_AirPressureID       = MFVarGetID (MDVarCommon_AirPressure,      "kPa",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutCommon_HumiditySpecificID = MFVarGetID (MDVarCommon_HumiditySpecific, "kg/kg", MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((MFModelAddFunction (_MDCommon_HumiditySpecific) == CMfailed))) return (CMfailed);
            break;
    }
    MFDefLeaving ("SpecificHumidity");
    return (_MDOutCommon_HumiditySpecificID);
}
