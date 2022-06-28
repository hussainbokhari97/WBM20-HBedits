/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDRainSMoistChg.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <string.h>
#include <math.h>
#include <MF.h>
#include <MD.h>

static float _MDSoilMoistALPHA = 5.0;

// Input
static int _MDInAirTemperatureID      = MFUnset;
static int _MDInCommon_PrecipID       = MFUnset;
static int _MDInPotETID               = MFUnset;
static int _MDInInterceptID           = MFUnset;
static int _MDInSnowPackID            = MFUnset;
static int _MDInSnowPackChgID         = MFUnset;
static int _MDInSoilAvailWaterCapID   = MFUnset;
static int _MDInIrrigation_AreaFracID = MFUnset;
// Output
static int _MDOutEvaptrsID            = MFUnset;
static int _MDOutSoilMoistCellID      = MFUnset;
static int _MDOutSoilMoistID          = MFUnset;
static int _MDOutSMoistChgID          = MFUnset;

static void _MDRainSMoistChg (int itemID) {	
// Input
	float airT         = MFVarGetFloat (_MDInAirTemperatureID,    itemID, 0.0); // Air temperature [degreeC]
	float precip       = MFVarGetFloat (_MDInCommon_PrecipID,     itemID, 0.0); // Precipitation [mm/dt]
	float pet          = MFVarGetFloat (_MDInPotETID,             itemID, 0.0); // Potential evapotranspiration [mm/dt]
	float snowPackChg  = MFVarGetFloat (_MDInSnowPackChgID,       itemID, 0.0); // Snow pack change [mm/dt]
	float irrAreaFrac  = _MDInIrrigation_AreaFracID != MFUnset ? MFVarGetFloat (_MDInIrrigation_AreaFracID, itemID, 0.0) : 0.0; // Irrigated area fraction
	float sMoist       = MFVarGetFloat (_MDOutSoilMoistID,        itemID, 0.0) / (1.0 - irrAreaFrac); // Soil moisture [mm]
	float awCap        = MFVarGetFloat (_MDInSoilAvailWaterCapID, itemID, 0.0); // Available water capacity
	float intercept    = _MDInInterceptID != MFUnset ? MFVarGetFloat (_MDInInterceptID, itemID, 0.0) : 0.0; // Interception (when the interception module is turned on) [mm/dt]
// Output
	float sMoistChg   = 0.0; // Soil moisture change [mm/dt]
	float evapotrans;

	if (awCap > 0.0) {
	// Input
		float snowPack = MFVarGetFloat (_MDInSnowPackID,          itemID, 0.0); // Snow pack [mm]
		if (snowPack > 0.0) sMoistChg = 0.0;
		else {
			float waterIn;
			waterIn = precip - snowPackChg - intercept;
			pet = pet > intercept ? pet - intercept : 0.0;
	    	if (waterIn > pet) { sMoistChg = waterIn - pet > awCap - sMoist ? awCap - sMoist : waterIn - pet; }
	    	else {
				float gm;
	        	gm = (1.0 - exp (- _MDSoilMoistALPHA * sMoist / awCap)) / (1.0 - exp (- _MDSoilMoistALPHA));
	        	sMoistChg = gm * (waterIn - pet);
		    	if (sMoist + sMoistChg > awCap) sMoistChg = awCap - sMoist;
		    	if (sMoist + sMoistChg <   0.0) sMoistChg =       - sMoist;
	    	}
	    	sMoist += sMoistChg;
		}
	} else sMoist = sMoistChg = 0.0;
	evapotrans = pet + intercept < precip - snowPackChg - sMoistChg ? pet + intercept : precip - snowPackChg - sMoistChg;
	MFVarSetFloat (_MDOutEvaptrsID,   itemID, evapotrans * (1.0 - irrAreaFrac));
	MFVarSetFloat (_MDOutSoilMoistID, itemID, sMoist     * (1.0 - irrAreaFrac));
	MFVarSetFloat (_MDOutSMoistChgID, itemID, sMoistChg  * (1.0 - irrAreaFrac));
}

int MDCore_RainSMoistChgDef () {
	int ret = 0;
	float par;
	const char *optStr;
	if (_MDOutSMoistChgID != MFUnset) return (_MDOutSMoistChgID);

	if ((optStr = MFOptionGet (MDParSoilMoistALPHA))  != (char *) NULL) {
		if (strcmp(optStr,MFhelpStr) == 0) CMmsgPrint (CMmsgInfo,"%s = %f",MDParSoilMoistALPHA, _MDSoilMoistALPHA);
		_MDSoilMoistALPHA = sscanf (optStr,"%f",&par) == 1 ? par : _MDSoilMoistALPHA;
	}
	
	MFDefEntering ("Rainfed Soil Moisture");

	if ((ret = MDIrrigation_GrossDemandDef()) == CMfailed) return (CMfailed);
	if ((ret != MFUnset) && ((_MDInIrrigation_AreaFracID = MDIrrigation_IrrAreaDef()) == CMfailed) ) return (CMfailed);

	if (((_MDInCommon_PrecipID     = MDCommon_PrecipitationDef ())   == CMfailed) ||
        ((_MDInSnowPackChgID       = MDCore_SnowPackChgDef ())       == CMfailed) ||
        ((_MDInPotETID             = MDCore_RainPotETDef ())         == CMfailed) ||
        ((_MDInInterceptID         = MDCore_RainInterceptDef ())     == CMfailed) ||
        ((_MDInSoilAvailWaterCapID = MDCore_SoilAvailWaterCapDef ()) == CMfailed) ||
        ((_MDInAirTemperatureID    = MDCommon_AirTemperatureDef ())  == CMfailed) ||
        ((_MDInSnowPackID          = MFVarGetID (MDVarCore_SnowPack,               "mm",  MFInput,  MFState, MFInitial))  == CMfailed) ||
        ((_MDOutEvaptrsID          = MFVarGetID (MDVarCore_RainEvapotranspiration, "mm",  MFOutput, MFFlux, MFBoundary))  == CMfailed) ||
        ((_MDOutSoilMoistID        = MFVarGetID (MDVarCore_RainSoilMoisture,       "mm",  MFOutput, MFState, MFInitial))  == CMfailed) ||
        ((_MDOutSMoistChgID        = MFVarGetID (MDVarCore_RainSoilMoistChange,    "mm",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDRainSMoistChg) == CMfailed)) return (CMfailed);
	MFDefLeaving ("Rainfed Soil Moisture");
	return (_MDOutSMoistChgID);
}

int MDCore_RainEvapotranspirationDef () { MDCore_RainSMoistChgDef (); return (_MDOutEvaptrsID); }
