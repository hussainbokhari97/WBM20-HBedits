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
static int _MDInCommon_HumidityDewPointID   = MFUnset;
static int _MDInCommon_SolarRadID           = MFUnset;
static int _MDInCommon_WindSpeedID          = MFUnset;
static int _MDInRouting_DischargeID         = MFUnset;
static int _MDInRouting_RiverWidthID        = MFUnset;
static int _MDInReservoir_StorageID         = MFUnset;
static int _MDInReservoir_ReleaseBottomID   = MFUnset;
static int _MDInReservoir_ReleaseSpillwayID = MFUnset;
static int _MDInWTemp_RiverTopID            = MFUnset;
static int _MDInWTemp_RiverBottomID         = MFUnset;
// Route
static int _MDOutWTemp_HeatFluxID           = MFUnset;
// Output
static int _MDOutWTemp_EquilTemp            = MFUnset;
static int _MDOutWTemp_EquilTempDiff        = MFUnset;
static int _MDOutWTemp_RiverID              = MFUnset;

#define MinTemp 1.0

static void _MDWTempRiver (int itemID) {
// Input
    float discharge       = MFVarGetFloat (_MDInRouting_DischargeID,      itemID, 0.0); // Outflowing discharge in m3/s
    float storage         = _MDInReservoir_StorageID != MFUnset ? MFVarGetFloat (_MDInReservoir_StorageID, itemID, 0.0) : 0.0;
    float wTempRiverTop   = MFVarGetFloat (_MDInWTemp_RiverTopID,         itemID, 0.0); // Outflowing discharge in m3/s
// Routed
    float heatFlux        = MFVarGetFloat (_MDOutWTemp_HeatFluxID,        itemID, 0.0); // Heat flux degC * m3/s
// Output
    float equilTemp;        // Equilibrium temperatur degC
    float equilTempDiff;  // Equilibrium temperature change in degC
    float wTempRiver;     // River temprature in degC
// Model
    float dt              = MFModelGet_dt ();          // Model time step in seconds
// Local

    if (storage > 0.0001) {
    // Input
        float reservoirReleaseBottom   = MFVarGetFloat (_MDInReservoir_ReleaseBottomID,   itemID, 0.0);
        float reservoirReleaseSpillway = MFVarGetFloat (_MDInReservoir_ReleaseSpillwayID, itemID, 0.0);
        float wTempRiverBottom         = MFVarGetFloat (_MDInWTemp_RiverBottomID,         itemID, 0.0);

        if (reservoirReleaseBottom + reservoirReleaseSpillway > 0.0)
            wTempRiver = wTempRiverBottom * reservoirReleaseBottom + wTempRiverTop * reservoirReleaseSpillway
                       / (reservoirReleaseBottom + reservoirReleaseSpillway);
        else wTempRiver = wTempRiverTop;
        equilTemp = wTempRiver;
        equilTempDiff = 0.0;
    } else {
    // Input
        float dewpointTemp = MFVarGetFloat (_MDInCommon_HumidityDewPointID, itemID, 0.0); // Dewpoint temperature in degC
     	float solarRad     = MFVarGetFloat (_MDInCommon_SolarRadID,         itemID, 0.0); // Solar radiation in W/m2 averaged over the day
        float windSpeed    = MFVarGetFloat (_MDInCommon_WindSpeedID,        itemID, 0.0); // Winds speed in m/s
        float channelWidth = MFVarGetFloat (_MDInRouting_RiverWidthID,      itemID, 0.0); // River width in m
    // Model
        float channelLength   = MFModelGetLength (itemID); // Channel length in m
        float cellArea        = MFModelGetArea   (itemID); // Cell area in m2
    // Local
        int i;
        float windFunc;
        float kay;
    
        wTempRiver = equilTemp = wTempRiverTop;
        windFunc = 9.2 + 0.46 * pow (windSpeed,2); // wind function
        for (i = 0; i < 4; ++i) {
            float meanTemp;
            float beta;
	        meanTemp  = (dewpointTemp + equilTemp) / 2; // mean of rivertemp initial and dew point
	        beta      = 0.35 + 0.015 * meanTemp + 0.0012 * pow (meanTemp, 2.0); //beta
	        kay       = 4.50 + 0.050 * equilTemp + (beta + 0.47) * windFunc; // K in W/m2/degC
	        equilTemp = dewpointTemp + solarRad / kay; // Solar radiation is in W/m2;
        }
        equilTempDiff = (equilTemp - wTempRiver) * (1.0 - exp (-kay * channelLength * channelWidth / (4181300 * discharge)));
        wTempRiver   += equilTempDiff;
        if (wTempRiver < MinTemp) { equilTempDiff += wTempRiver - MinTemp; wTempRiver = MinTemp; }
    }
    MFVarSetFloat(_MDOutWTemp_EquilTemp,         itemID, equilTemp);
    MFVarSetFloat(_MDOutWTemp_EquilTempDiff,     itemID, equilTempDiff);
    MFVarSetFloat(_MDOutWTemp_RiverID,           itemID, wTempRiver);
    MFVarSetFloat(_MDOutWTemp_HeatFluxID,        itemID, wTempRiver * discharge * dt); // Route
}

int MDWTemp_RiverDef () {

	if (_MDOutWTemp_RiverID != MFUnset) return (_MDOutWTemp_RiverID);

	MFDefEntering ("Route river temperature");
	if (((_MDInRouting_DischargeID          = MDRouting_DischargeDef ())                  == CMfailed) ||
        ((_MDInReservoir_StorageID          = MDReservoir_StorageDef ())                  == CMfailed) ||
        ((_MDInReservoir_StorageID != MFUnset) &&
         (((_MDInReservoir_ReleaseBottomID   = MDReservoir_ReleaseBottomDef ())           == CMfailed)   ||
          ((_MDInReservoir_ReleaseSpillwayID = MDReservoir_ReleaseSpillwayDef ())         == CMfailed))) ||
        ((_MDInCommon_HumidityDewPointID    = MDCommon_HumidityDewPointTemperatureDef ()) == CMfailed) ||
        ((_MDInCommon_SolarRadID            = MDCommon_SolarRadDef ())                    == CMfailed) ||
        ((_MDInWTemp_RiverBottomID          = MDWTemp_RiverBottomDef ())                  == CMfailed) ||
        ((_MDInWTemp_RiverTopID             = MDWTemp_RiverTopDef ())                     == CMfailed) ||
        ((_MDInRouting_RiverWidthID         = MDRouting_RiverWidthDef ())                 == CMfailed) ||
        ((_MDInCommon_WindSpeedID           = MFVarGetID (MDVarCommon_WindSpeed,        "m/s",       MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutWTemp_HeatFluxID            = MFVarGetID (MDVarWTemp_HeatFlux,          "degC*m3/s", MFRoute,  MFState, MFInitial))  == CMfailed) ||
        ((_MDOutWTemp_EquilTemp   	        = MFVarGetID (MDVarWTemp_EquilTemp,         "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutWTemp_EquilTempDiff   	    = MFVarGetID (MDVarWTemp_EquilTempDiff,     "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutWTemp_RiverID               = MFVarGetID (MDVarWTemp_River,             "degC",      MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempRiver) == CMfailed)) return (CMfailed);
	   MFDefLeaving ("Route river temperature");
	   return (_MDOutWTemp_RiverID);
}