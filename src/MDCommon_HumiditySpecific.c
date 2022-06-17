/******************************************************************************
 GHAAS Water Balance/Transport Model
 Global Hydrological Archive and Analysis System
 Copyright 1994-2022, UNH - ASRC/CUNY

 MDCommon_HumiditySpecific.c

 amiara@ccny.cuny.edu

 *******************************************************************************/

/********************************************************************************
 * Calculates Specific Humidity from pressure, air temperature and rh

 * ******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_HumiditySaturatedVaporPressID = MFUnset;
static int _MDInCommon_HumidityRelativeID            = MFUnset;
static int _MDInCommon_AirPressureID                 = MFUnset;

// Output
static int _MDOutCommon_HumiditySpecificID = MFUnset;

static void _MDCommon_HumiditySpecific (int itemID) {
// Input
    float saturatedVP;      // Saturated vapor pressure in Pa
    float relativeHumidity; // Relative humidity in percent
    float airPressure;      // Air pressure in Pa
// Output
    float specifichumidity; // Specific humidity in percent

    airPressure      = MFVarGetFloat (_MDInCommon_AirPressureID,                 itemID, 0.0);
    saturatedVP      = MFVarGetFloat (_MDInCommon_HumiditySaturatedVaporPressID, itemID, 0.0);
    relativeHumidity = MFVarGetFloat (_MDInCommon_HumidityRelativeID,            itemID, 0.0) / 100.0;

    specifichumidity = relativeHumidity * saturatedVP / (airPressure - relativeHumidity * saturatedVP);
    MFVarSetFloat(_MDOutCommon_HumiditySpecificID, itemID, specifichumidity);
}

int MDCommon_HumiditySpecificDef () {
    int optID = MFinput;
    const char *optStr;
    
    if (_MDOutCommon_HumiditySpecificID != MFUnset) return (_MDOutCommon_HumiditySpecificID);
    
    MFDefEntering ("SpecificHumidity");
    if ((optStr = MFOptionGet (MDOptWeather_SpecificHumidity)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
        switch (optID) {
            default:      MFOptionMessage (MDOptWeather_SpecificHumidity, optStr, MFsourceOptions); return (CMfailed);
            case MFhelp:  MFOptionMessage (MDOptWeather_SpecificHumidity, optStr, MFsourceOptions);
            case MFinput: _MDOutCommon_HumiditySpecificID = MFVarGetID (MDVarCommon_HumiditySpecific, "%",      MFInput, MFState, MFBoundary); break;
            case MFcalculate:
                if (((_MDInCommon_HumiditySaturatedVaporPressID  = MDCommon_HumiditySaturatedVaporPressureDef ()) == CMfailed) ||
                    ((_MDInCommon_HumidityRelativeID             = MDCommon_HumidityRelativeDef())                == CMfailed) ||
                    ((_MDInCommon_AirPressureID       = MFVarGetID (MDVarCommon_AirPressure,      "kPa",  MFInput, MFState, MFBoundary)) == CMfailed) ||
                    ((_MDOutCommon_HumiditySpecificID = MFVarGetID (MDVarCommon_HumiditySpecific, "%",    MFOutput, MFState, MFBoundary)) == CMfailed) ||
                    ((MFModelAddFunction (_MDCommon_HumiditySpecific) == CMfailed))) return (CMfailed);
                break;
        }
    MFDefLeaving ("SpecificHumidity");
    return (_MDOutCommon_HumiditySpecificID);
}
