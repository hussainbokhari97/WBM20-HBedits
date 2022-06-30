/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDTP2M_TempGrdWater.c

wil.wollheim@unh.edu

EDITED: amiara@ccny.cuny.edu
EDITED: ariel.miara@nrel.gov Feb11 2021

Calculate groundwater temperature by mixing existing groundwater, rain recharge, and irrigation return flow.
Rain recharge temperature is calculated in MDWTempSurfRunoff
Irrigation return flow is assumed to have air temperature.
*******************************************************************************/

#include <string.h>
#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_AirTemperatureID = MFUnset;
static int _MDInTP2M_GW_TempID   = MFUnset;
// Output
static int _MDOutTP2M_WTempGrdWaterID  = MFUnset;

static void _MDWTP2M_TempGrdWater (int itemID) {
    MFVarSetFloat (_MDOutTP2M_WTempGrdWaterID, itemID, MDMaximum (MFVarGetFloat (_MDInCommon_AirTemperatureID, itemID, 0.0), 5.0));
}

int MDTP2M_WTempGrdWaterDef () {

	if (_MDOutTP2M_WTempGrdWaterID != MFUnset) return (_MDOutTP2M_WTempGrdWaterID);

	MFDefEntering ("Groundwater temperature");

	if (((_MDInCommon_AirTemperatureID = MDCommon_AirTemperatureDef ()) == CMfailed) ||
        ((_MDOutTP2M_WTempGrdWaterID   = MFVarGetID (MDVarTP2M_WTempGrdWater, "degC", MFOutput, MFState, MFInitial))  == CMfailed) ||
		(MFModelAddFunction(_MDWTP2M_TempGrdWater) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Groundwater temperature");
	return (_MDOutTP2M_WTempGrdWaterID);
}
