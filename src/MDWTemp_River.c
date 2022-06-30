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
static int _MDInWTemp_RunoffID         = MFUnset;
static int _MDInWTemp_HeatFluxID            = MFUnset; // Route
// Output
static int _MDOutWTemp_Equil_Temp           = MFUnset;
static int _MDOutWTemp_RiverID         = MFUnset;


static void _MDWTempRiver (int itemID) {
// Model
    float dt            = MFModelGet_dt ();
    float channelLength = MFModelGetLength (itemID);
    float cellArea      = MFModelGetArea (itemID);
// Input
    float discharge0 = MFVarGetFloat (_MDInRouting_Discharge0ID, itemID, 0.0); // Outflowing discharge in m3/s 
    float discharge  = MFVarGetFloat (_MDInRouting_DischargeID,  itemID, 0.0); // Outflowing discharge in m3/s 
   	float runoffTemp = MFVarGetFloat (_MDInWTemp_RunoffID,   itemID, 0.0); // Runoff temperature degC
// Routed
    float heatFlux   = MFVarGetFloat (_MDInWTemp_HeatFluxID,      itemID, 0.0); // Heat flux degC * m3/s
// Output
    float riverTemp; // River temprature in degC
    float equilTemp; // Equlibrium temperature in degC

    if ((discharge0 > 0.0) && (discharge > 0.0) && (heatFlux > 0.0)) { 
        // Input
        float dewpointTemp = MFVarGetFloat (_MDInCommon_HumidityDewPointID, itemID, 0.0); // Dewpoint temperature in degC
        float runoffVolume = MFVarGetFloat (_MDInCore_RunoffVolumeID,       itemID, 0.0); // RO volume in m3/s

        heatFlux += runoffTemp * runoffVolume;
        riverTemp = equilTemp = heatFlux / discharge0;
        // EQUILIBRIUM TEMP MODEL - Edinger et al. 1974: Heat Exchange and Transport in the Environment
        if (dewpointTemp > 0.0) {
            // Input
     	    float solarRad     = MFVarGetFloat (_MDInCommon_SolarRadID,    itemID, 0.0); // Solar radiation in W/m2 averaged over the day
        	float windSpeed    = MFVarGetFloat (_MDInCommon_WindSpeedID,   itemID, 0.0); // Winds speed in m/s
           	float channelWidth = MFVarGetFloat (_MDInRouting_RiverWidthID, itemID, 0.0); // River width in m
            // Local
            int i;
            float windFunc;
            float kay;

            windFunc = 9.2 + 0.46 * pow (windSpeed,2); // wind function
            for (i = 0; i < 4; ++i) {
                float meanTemp;
                float beta;
	            meanTemp  = (dewpointTemp + equilTemp) / 2; // mean of rivertemp initial and dew point
	            beta      = 0.35 + 0.015 * meanTemp + 0.0012 * pow (meanTemp, 2.0); //beta
	            kay       = 4.50 + 0.050 * equilTemp + (beta + 0.47) * windFunc; // K in W/m2/degC
	            equilTemp = dewpointTemp + solarRad / kay; // Solar radiation is in W/m2;
            }
            riverTemp = equilTemp + (riverTemp - equilTemp) * exp (-kay * channelLength * channelWidth / (4181.3 * discharge * dt));
        }
    } else  riverTemp = equilTemp = runoffTemp;

    heatFlux = riverTemp * discharge;
    MFVarSetFloat(_MDOutWTemp_Equil_Temp,   itemID, equilTemp);
    MFVarSetFloat(_MDInWTemp_HeatFluxID,    itemID, heatFlux);  // Route
    MFVarSetFloat(_MDOutWTemp_RiverID, itemID, riverTemp);
}

int MDWTemp_RiverDef () {

	if (_MDOutWTemp_RiverID != MFUnset) return (_MDOutWTemp_RiverID);

	MFDefEntering ("Route river temperature");
	if (((_MDInCommon_HumidityDewPointID = MDCommon_HumidityDewPointTemperatureDef ()) == CMfailed) ||
        ((_MDInCommon_SolarRadID         = MDCommon_SolarRadDef ())                    == CMfailed) ||
        ((_MDInCore_RunoffVolumeID       = MDCore_RunoffVolumeDef ())                  == CMfailed) ||
        ((_MDInRouting_DischargeID       = MDRouting_DischargeDef ())                  == CMfailed) ||
        ((_MDInWTemp_RunoffID            = MDWTemp_RunoffDef ())                       == CMfailed) ||
        ((_MDInRouting_RiverWidthID      = MDRouting_RiverWidthDef ())                 == CMfailed) ||
        ((_MDInRouting_Discharge0ID      = MFVarGetID (MDVarRouting_Discharge0, "m3/s",    MFInput,  MFState, MFInitial))  == CMfailed) ||
        ((_MDInCommon_WindSpeedID        = MFVarGetID (MDVarCommon_WindSpeed,   "m/s",     MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInWTemp_HeatFluxID          = MFVarGetID (MDVarWTemp_HeatFlux,     "m3*degC", MFRoute,  MFFlux,  MFInitial))  == CMfailed) ||
        ((_MDOutWTemp_Equil_Temp   	     = MFVarGetID (MDVarWTemp_EquilTemp,    "degC",    MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutWTemp_RiverID            = MFVarGetID (MDVarWTemp_River,        "degC",    MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempRiver) == CMfailed)) return (CMfailed);
	   MFDefLeaving ("Route river temperature");
	   return (_MDOutWTemp_RiverID);
}