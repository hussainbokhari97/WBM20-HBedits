/******************************************************************************
GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDTP2M_WTempRiver.c

wil.wollheim@unh.edu

amiara@ccny.cuny.edu - updated river temperature calculations as of Sep 2016

Route temperature through river network

*******************************************************************************/

#include <string.h>
#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_HumidityDewPointID  = MFUnset; // FOR NEW TEMP MODULE
static int _MDInCommon_SolarRadID          = MFUnset;
static int _MDInCommon_WindSpeedID         = MFUnset;
static int _MDInCore_RunoffVolumeID        = MFUnset;
static int _MDInRouting_Discharge0ID       = MFUnset;
static int _MDInRouting_DischargeID        = MFUnset;
static int _MDInRouting_RiverWidthID       = MFUnset;
static int _MDInTP2M_WTempRunoffID         = MFUnset;
static int _MDInTP2M_HeatFluxID            = MFUnset; // Route
// Output
static int _MDOutTP2M_Equil_Temp           = MFUnset;
static int _MDOutTP2M_WTempRiverID         = MFUnset;


static void _MDWTempRiver (int itemID) {
// Input
    float dewpointTemp; // Dewpoint temperature in degC
 	float solarRad;     // Solar radiation in W/m2 averaged over the day
 	float windSpeed;    // Winds speed in m/s
    float runoffVolume; // RO volume in m3/s
    float discharge0;   // Outflowing discharge in m3/s 
    float discharge;    // Outflowing discharge in m3/s 
   	float channelWidth; // River width in m
   	float runoffTemp;   // Runoff temperature degC
// Routed
    float heatFlux;     // Heat flux degC * m3/s
// Output
    float riverTemp;    // River temprature in degC
// Local
    float equilTemp;    // Equlibrium temperature in degC
    float dt = MFModelGet_dt ();

    dewpointTemp    = MFVarGetFloat (_MDInCommon_HumidityDewPointID,   itemID, 0.0);
 	solarRad        = MFVarGetFloat (_MDInCommon_SolarRadID,           itemID, 0.0);
 	windSpeed       = MFVarGetFloat (_MDInCommon_WindSpeedID,          itemID, 0.0);
    runoffVolume    = MFVarGetFloat (_MDInCore_RunoffVolumeID,         itemID, 0.0);
    discharge0      = MFVarGetFloat (_MDInRouting_Discharge0ID,        itemID, 0.0);
    discharge       = MFVarGetFloat (_MDInRouting_DischargeID,         itemID, 0.0);
   	channelWidth    = MFVarGetFloat (_MDInRouting_RiverWidthID,        itemID, 0.0);
   	runoffTemp      = MFVarGetFloat (_MDInTP2M_WTempRunoffID,          itemID, 0.0);
    heatFlux        = MFVarGetFloat (_MDInTP2M_HeatFluxID,             itemID, 0.0);

    if ((discharge0 > 0.001) && (discharge > 0.001)) {
        heatFlux += runoffTemp * runoffVolume;
        riverTemp = equilTemp = heatFlux / discharge0;
        // EQUILIBRIUM TEMP MODEL - Edinger et al. 1974: Heat Exchange and Transport in the Environment
        if (dewpointTemp > 0.0) {
            int i;
            float windFunc;
            float meanTemp;
            float beta;
            float kay;
            float channelLength = MFModelGetLength(itemID);

            windFunc = 9.2 + 0.46 * pow (windSpeed,2); // wind function
            for (i = 0; i < 4; ++i) {
	            meanTemp  = (dewpointTemp + equilTemp) / 2; // mean of rivertemp initial and dew point
	            beta      = 0.35 + 0.015 * meanTemp + 0.0012 * pow (meanTemp, 2.0); //beta
	            kay       = 4.50 + 0.050 * equilTemp + (beta + 0.47) * windFunc; // K in W/m2/degC
	            equilTemp = dewpointTemp + solarRad / kay; // Solar radiation is in W/m2;
            }
            equilTemp += (riverTemp - equilTemp) * exp (-kay * channelLength * channelWidth / (4181.3 * discharge * dt));
            riverTemp  = equilTemp;
            riverTemp  = riverTemp > dewpointTemp ? MDMinimum (riverTemp, equilTemp) : MDMaximum (dewpointTemp, equilTemp);
        }
   	} else
        riverTemp = equilTemp = runoffTemp;

    heatFlux = riverTemp * discharge;
    MFVarSetFloat(_MDOutTP2M_Equil_Temp,   itemID, equilTemp);
    MFVarSetFloat(_MDInTP2M_HeatFluxID,    itemID, heatFlux);  // Route
    MFVarSetFloat(_MDOutTP2M_WTempRiverID, itemID, riverTemp);
}

int MDTP2M_WTempRiverDef () {

	if (_MDOutTP2M_WTempRiverID != MFUnset) return (_MDOutTP2M_WTempRiverID);

	MFDefEntering ("Route river temperature");
	if (((_MDInCommon_HumidityDewPointID = MDCommon_HumidityDewPointTemperatureDef ()) == CMfailed) ||
        ((_MDInCommon_SolarRadID         = MDCommon_SolarRadDef ())                    == CMfailed) ||
        ((_MDInCore_RunoffVolumeID       = MDCore_RunoffVolumeDef ())                  == CMfailed) ||
        ((_MDInRouting_DischargeID       = MDRouting_DischargeDef ())                  == CMfailed) ||
        ((_MDInTP2M_WTempRunoffID        = MDTP2M_WTempRunoffDef ())                   == CMfailed) ||
        ((_MDInRouting_RiverWidthID      = MDRouting_RiverWidthDef ())                 == CMfailed) ||
        ((_MDInRouting_Discharge0ID      = MFVarGetID (MDVarRouting_Discharge0,      "m3/s",      MFInput,  MFState, MFInitial))  == CMfailed) ||
        ((_MDInCommon_WindSpeedID        = MFVarGetID (MDVarCommon_WindSpeed,        "m/s",       MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInTP2M_HeatFluxID           = MFVarGetID (MDVarTP2M_HeatFlux,           "m3*degC/d", MFRoute,  MFFlux,  MFBoundary)) == CMfailed) ||
        ((_MDOutTP2M_Equil_Temp   	     = MFVarGetID (MDVarTP2M_Equil_Temp,         "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutTP2M_WTempRiverID        = MFVarGetID (MDVarTP2M_WTempRiver,         "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempRiver) == CMfailed)) return (CMfailed);
	   MFDefLeaving ("Route river temperature");
	   return (_MDOutTP2M_WTempRiverID);
}