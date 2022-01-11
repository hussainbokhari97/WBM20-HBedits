/******************************************************************************
 GHAAS Water Balance/Transport Model
 Global Hydrological Archive and Analysis System
 Copyright 1994-2022, UNH - ASRC/CUNY

 MDCommon_HumidityRelative.c

 amiara@ccny.cuny.edu

 *******************************************************************************/

/********************************************************************************
 * Calculates Specific Humidity from pressure, air temperature and rh
 * ******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_AirTemperatureID    = MFUnset;
static int _MDInCommon_HumiditySpecificID  = MFUnset;
static int _MDInCommon_AirPressureID       = MFUnset;
// Output
static int _MDOutCommon_HumidityRelativeID = MFUnset;

static void _MDRelativeHumidity (int itemID) {
    float airtemp;
    float sh;
    float airpressure;
    float es;
    float e;
    float relativehumidity;
    float v1;
    float v2;

    airtemp     = MFVarGetFloat (_MDInCommon_AirTemperatureID,   itemID, 0.0) + 273.16;
    airpressure = MFVarGetFloat (_MDInCommon_AirPressureID,      itemID, 0.0); //pressure (Pa)
    sh          = MFVarGetFloat (_MDInCommon_HumiditySpecificID, itemID, 0.0); // jan27 2017 - divide by 100 for NCEP

    v1 = exp( (17.67 * (airtemp - 273.16) / (airtemp - 29.65) ) );
    v2 = 0.263 * airpressure;
    relativehumidity = (sh * v2)/v1;
    MFVarSetFloat(_MDOutCommon_HumidityRelativeID, itemID, relativehumidity);
}

int MDCommon_HumidityRelativeDef () {
    int optID = MFinput;
    const char *optStr;
     
    if (_MDOutCommon_HumidityRelativeID != MFUnset) return (_MDOutCommon_HumidityRelativeID);    

    MFDefEntering ("RelativeHumidity");
    if ((optStr = MFOptionGet(MDOptWeather_RelativeHumidity)) != (char *) NULL) optID = CMoptLookup(MFsourceOptions, optStr, true);
    switch (optID) {
        default:      MFOptionMessage (MDOptWeather_RelativeHumidity, optStr, MFsourceOptions); return (CMfailed);
        case MFhelp:  MFOptionMessage (MDOptWeather_RelativeHumidity, optStr, MFsourceOptions);
        case MFinput: _MDOutCommon_HumidityRelativeID = MFVarGetID (MDVarCommon_HumidityRelative, "degC", MFInput, MFState, MFBoundary); break;
        case MFcalculate:
            if (((_MDInCommon_AirTemperatureID    = MDCommon_AirTemperatureDef ()) == CMfailed) ||
                ((_MDInCommon_AirPressureID       = MFVarGetID (MDVarCommon_AirPressure,      "kPa",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDInCommon_HumiditySpecificID  = MFVarGetID (MDVarCommon_HumiditySpecific, "%",    MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutCommon_HumidityRelativeID = MFVarGetID (MDVarCommon_HumidityRelative, "%",    MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((MFModelAddFunction (_MDRelativeHumidity) == CMfailed))) return (CMfailed);
            break;
    }
    MFDefLeaving ("RelativeHumidity");
    return (_MDOutCommon_HumidityRelativeID);
}
