/******************************************************************************
GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDWTemp_River.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <string.h>
#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_HumidityDewPointID  = MFUnset;
static int _MDInCommon_SolarRadID          = MFUnset;
static int _MDInCommon_WindSpeedID         = MFUnset;
static int _MDInCore_RunoffFlowID          = MFUnset;
static int _MDInRouting_Discharge0ID       = MFUnset;
static int _MDInRouting_DischargeID        = MFUnset;
static int _MDInRouting_RiverWidthID       = MFUnset;
static int _MDInWTemp_RunoffID             = MFUnset;
// Route
static int _MDOutWTemp_HeatFluxNoEquilID   = MFUnset;
static int _MDOutWTemp_HeatFluxID          = MFUnset;
// Output
static int _MDOutWTemp_EquilTemp           = MFUnset;
static int _MDOutWTemp_EquilTempDiff       = MFUnset;
static int _MDOutWTemp_RiverNoEquilID      = MFUnset;
static int _MDOutWTemp_RiverID             = MFUnset;

static void _MDWTempRiver (int itemID) {
// Model
    float dt              = MFModelGet_dt ();          // Model time step in seconds
    float channelLength   = MFModelGetLength (itemID); // Channel length in m
    float cellArea        = MFModelGetArea (itemID);   // Cell area in m2
// Input
    float discharge0      = MFVarGetFloat (_MDInRouting_Discharge0ID,     itemID, 0.0); // Outflowing discharge in m3/s 
    float discharge       = MFVarGetFloat (_MDInRouting_DischargeID,      itemID, 0.0); // Outflowing discharge in m3/s 
    float runoffFlow      = MFVarGetFloat (_MDInCore_RunoffFlowID,        itemID, 0.0); // RO volume in m3/s
   	float runoffTemp      = MFVarGetFloat (_MDInWTemp_RunoffID,           itemID, 0.0); // Runoff temperature degC
// Routed
    float heatFlux        = MFVarGetFloat (_MDOutWTemp_HeatFluxID,        itemID, 0.0); // Heat flux degC * m3/s
    float heatFluxNoEquil = MFVarGetFloat (_MDOutWTemp_HeatFluxNoEquilID, itemID, 0.0); // Heat flux with equilibration degC * m3/s
// Output
    float equilTemp;        // Equilibrium temperatur degC
    float equilTempDiff;    // Equilibrium temperature change in degC
    float riverTempNoEquil; // River temperature without equilibration in degC
    float riverTemp;        // River temprature in degC
// Local
    float flowThreshold = cellArea * 0.0001 / dt; // 0.1 mm/day over the the cell area

    if ((discharge0 > runoffFlow) && (discharge0 > flowThreshold) && (discharge > flowThreshold)) { 
    // Input
        float dewpointTemp = MFVarGetFloat (_MDInCommon_HumidityDewPointID, itemID, 0.0); // Dewpoint temperature in degC
     	float solarRad     = MFVarGetFloat (_MDInCommon_SolarRadID,         itemID, 0.0); // Solar radiation in W/m2 averaged over the day
        float windSpeed    = MFVarGetFloat (_MDInCommon_WindSpeedID,        itemID, 0.0); // Winds speed in m/s
        float channelWidth = MFVarGetFloat (_MDInRouting_RiverWidthID,      itemID, 0.0); // River width in m
    // Local
        int i;
        float windFunc;
        float kay;

        heatFlux        += runoffTemp * runoffFlow;
        heatFluxNoEquil += runoffTemp * runoffFlow;
        riverTemp = equilTemp = heatFlux / discharge0;
        riverTempNoEquil = heatFluxNoEquil / discharge0;
        if (riverTemp > 50.0) {
            CMmsgPrint (CMmsgWarning, "Day: %3d Cell: %10ld River Temperature: %6.1f\n", MFDateGetDayOfYear (), itemID, riverTemp);
            riverTemp = equilTemp = runoffTemp;
        }
        // EQUILIBRIUM TEMP MODEL - Edinger et al. 1974: Heat Exchange and Transport in the Environment
        windFunc = 9.2 + 0.46 * pow (windSpeed,2); // wind function
        for (i = 0; i < 4; ++i) {
            float meanTemp;
            float beta;
	        meanTemp  = (dewpointTemp + equilTemp) / 2; // mean of rivertemp initial and dew point
	        beta      = 0.35 + 0.015 * meanTemp + 0.0012 * pow (meanTemp, 2.0); //beta
	        kay       = 4.50 + 0.050 * equilTemp + (beta + 0.47) * windFunc; // K in W/m2/degC
	        equilTemp = dewpointTemp + solarRad / kay; // Solar radiation is in W/m2;
        }
        if (equilTemp < 0.0) equilTemp = 0.0;
        equilTempDiff = (equilTemp - riverTemp) * (1.0 - exp (-kay * channelLength * channelWidth / (4.1813 * discharge)));
        riverTemp += equilTempDiff;
        if (riverTemp < 0.0) { equilTempDiff += riverTemp; riverTemp = 0.0; }
    } else {
        riverTemp = riverTempNoEquil = equilTemp = runoffTemp;
        equilTempDiff = 0.0;
    }
    MFVarSetFloat(_MDOutWTemp_EquilTemp,         itemID, equilTemp);
    MFVarSetFloat(_MDOutWTemp_EquilTempDiff,     itemID, equilTempDiff);
    MFVarSetFloat(_MDOutWTemp_RiverNoEquilID,    itemID, riverTempNoEquil);
    MFVarSetFloat(_MDOutWTemp_RiverID,           itemID, riverTemp);
    MFVarSetFloat(_MDOutWTemp_HeatFluxNoEquilID, itemID, riverTempNoEquil * discharge); // Route
    MFVarSetFloat(_MDOutWTemp_HeatFluxID,        itemID, riverTemp        * discharge); // Route
}

int MDWTemp_RiverDef () {

	if (_MDOutWTemp_RiverID != MFUnset) return (_MDOutWTemp_RiverID);

	MFDefEntering ("Route river temperature");
	if (((_MDInCommon_HumidityDewPointID = MDCommon_HumidityDewPointTemperatureDef ()) == CMfailed) ||
        ((_MDInCommon_SolarRadID         = MDCommon_SolarRadDef ())                    == CMfailed) ||
        ((_MDInCore_RunoffFlowID         = MDCore_RunoffFlowDef ())                    == CMfailed) ||
        ((_MDInRouting_DischargeID       = MDRouting_DischargeDef ())                  == CMfailed) ||
        ((_MDInWTemp_RunoffID            = MDWTemp_RunoffDef ())                       == CMfailed) ||
        ((_MDInRouting_RiverWidthID      = MDRouting_RiverWidthDef ())                 == CMfailed) ||
        ((_MDInRouting_Discharge0ID      = MFVarGetID (MDVarRouting_Discharge0,      "m3/s",      MFInput,  MFState, MFInitial))  == CMfailed) ||
        ((_MDInCommon_WindSpeedID        = MFVarGetID (MDVarCommon_WindSpeed,        "m/s",       MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutWTemp_HeatFluxID         = MFVarGetID (MDVarWTemp_HeatFlux,          "degC*m3/s", MFRoute,  MFState, MFInitial))  == CMfailed) ||
        ((_MDOutWTemp_HeatFluxNoEquilID  = MFVarGetID (MDVarWTemp_HeatFluxNoEquil,   "degC*m3/s", MFRoute,  MFState, MFInitial))  == CMfailed) ||
        ((_MDOutWTemp_EquilTemp   	     = MFVarGetID (MDVarWTemp_EquilTemp,         "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutWTemp_EquilTempDiff   	 = MFVarGetID (MDVarWTemp_EquilTempDiff,     "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutWTemp_RiverNoEquilID     = MFVarGetID (MDVarWTemp_RiverNoEquil,      "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutWTemp_RiverID            = MFVarGetID (MDVarWTemp_River,             "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempRiver) == CMfailed)) return (CMfailed);
	   MFDefLeaving ("Route river temperature");
	   return (_MDOutWTemp_RiverID);
}