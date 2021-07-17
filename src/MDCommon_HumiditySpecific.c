/******************************************************************************
 GHAAS Water Balance/Transport Model
 Global Hydrological Archive and Analysis System
 Copyright 1994-2021, UNH - ASRC/CUNY

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
static int _MDInCommon_AirTemperatureID = MFUnset;
static int _MDInCommon_HumidityRelativeID = MFUnset;
static int _MDInCommon_AirPressureID = MFUnset;

// Output
static int _MDOutCommon_HumiditySpecificID = MFUnset;

static void _MDSpecificHumidity(int itemID) {
    float airtemp;
    float rh;
    float airpressure;
    float es;
    float e;
    float specifichumidity;
    float v1;
    float v2;

    airtemp          = MFVarGetFloat(_MDInCommon_AirTemperatureID,   itemID, 0.0) + 273.16;
    airpressure      = MFVarGetFloat(_MDInCommon_AirPressureID,      itemID, 0.0); //pressure (Pa)

    rh = MFVarGetFloat(_MDInCommon_HumidityRelativeID, itemID, 0.0); // %

    v1 = exp( (17.67 * (airtemp - 273.16) / (airtemp - 29.65) ) );
    v2 = 0.263 * airpressure;
    specifichumidity = (rh * v1) / v2;
    MFVarSetFloat(_MDOutCommon_HumiditySpecificID, itemID, specifichumidity);
}

enum { MDinput, MDcalculate };

int MDCommon_HumiditySpecificDef () {
    int optID = MDinput;
    const char *optStr, *optName = MDOptWeather_SpecificHumidity;
    const char *options [] = { MDNoneStr, MDInputStr, MDCalculateStr, (char *) NULL};
    
    if (_MDOutCommon_HumiditySpecificID != MFUnset) return (_MDOutCommon_HumiditySpecificID);
    
    MFDefEntering ("SpecificHumidity");
    if ((optStr = MFOptionGet (optName)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
        switch (optID) {
        case MDinput:
            if ((_MDOutCommon_HumiditySpecificID = MFVarGetID (MDVarCommon_HumiditySpecific, "%",      MFInput, MFState, MFBoundary)) == CMfailed) return (CMfailed);
            break;
        case MDcalculate:
            if (((_MDInCommon_HumidityRelativeID  = MDCommon_HumidityRelativeDef()) == CMfailed) ||
                ((_MDInCommon_AirTemperatureID    = MDCommon_AirTemperatureDef ())  == CMfailed) ||
                ((_MDInCommon_AirPressureID       = MFVarGetID (MDVarCommon_AirPressure,      "kPa",  MFInput, MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutCommon_HumiditySpecificID = MFVarGetID (MDVarCommon_HumiditySpecific, "%",    MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((MFModelAddFunction (_MDSpecificHumidity) == CMfailed))) return (CMfailed);
            break;
        default: MFOptionMessage (optName, optStr, options); return (CMfailed);
    }
    MFDefLeaving ("SpecificHumidity");
    return (_MDOutCommon_HumiditySpecificID);
}
