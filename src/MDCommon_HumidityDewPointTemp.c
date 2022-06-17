/******************************************************************************
 GHAAS Water Balance/Transport Model
 Global Hydrological Archive and Analysis System
 Copyright 1994-2022, UNH - ASRC/CUNY

 MDCommon_HumidityDewPointTemp.c

 amiara@ccny.cuny.edu

 *******************************************************************************/

/********************************************************************************
 * Calculates Specific Humidity from pressure, air temperature and rh
 * ******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_HumidityVaporPressureID = MFUnset;
// Output
static int _MDOutCommon_HumidityDewPointTempID = MFUnset;

static void _MDCommon_HumidityDewPointTemp (int itemID) {
// Input
    float vaporPressure;  // Vapor pressure in Pa
    float dewPointTemp;   // Dew point temperature in degC

    vaporPressure = MFVarGetFloat (_MDInCommon_HumidityVaporPressureID,   itemID, 0.0);
    dewPointTemp  = vaporPressure > 611 ? 237.3 * log (vaporPressure / 611) / (17.27 - log (vaporPressure / 611.0))  // Over water 
                                        : 265.5 * log (vaporPressure / 611) / (21.87 - log (vaporPressure / 611.0)); // Over ice
    MFVarSetFloat(_MDOutCommon_HumidityDewPointTempID, itemID, dewPointTemp);
}

int MDCommon_HumidityDewPointTemperatureDef () {
    int optID = MFinput;
    const char *optStr;

    if (_MDOutCommon_HumidityDewPointTempID != MFUnset) return (_MDOutCommon_HumidityDewPointTempID);    

    MFDefEntering ("DewPointTemperature");
    if ((optStr = MFOptionGet(MDVarCommon_HumidityDewPointTemperature)) != (char *) NULL) optID = CMoptLookup(MFsourceOptions, optStr, true);
    switch (optID) {
        default:
        case MFhelp: MFOptionMessage (MDVarCommon_HumidityDewPointTemperature, optStr, MFsourceOptions); return (CMfailed);
        case MFinput: _MDOutCommon_HumidityDewPointTempID = MFVarGetID (MDVarCommon_HumidityDewPointTemperature, "degC", MFInput, MFState, MFBoundary); break;
        case MFcalculate:
            if (((_MDInCommon_HumidityVaporPressureID = MDCommon_HumidityVaporPressureDef ()) == CMfailed) ||
                ((MFModelAddFunction (_MDCommon_HumidityDewPointTemp) == CMfailed))) return (CMfailed);
            break;
    }
    MFDefLeaving ("DewPointTemperature");
    return (_MDOutCommon_HumidityDewPointTempID);
}
