/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDWTemp_GrdWater.c

bfekete@ccny.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_AirTemperatureID  = MFUnset;
static int _MDInAux_AirTemperatureMeanID = MFUnset;
// Output
static int _MDOutWTemp_GrdWaterID        = MFUnset;

static void _MDWTemp_GrdWater (int itemID) {
// Input
	float airTemp     = MFVarGetFloat (_MDInCommon_AirTemperatureID,  itemID, 0.0); // Air temperature degC

    MFVarSetFloat (_MDOutWTemp_GrdWaterID, itemID,  MDMaximum(airTemp, 5.0));
}

int MDWTemp_GrdWaterDef () {

	if (_MDOutWTemp_GrdWaterID != MFUnset) return (_MDOutWTemp_GrdWaterID);

	MFDefEntering ("Groundwater temperature");

	if (((_MDInCommon_AirTemperatureID = MDCommon_AirTemperatureDef  ()) == CMfailed) ||
        ((_MDOutWTemp_GrdWaterID       = MFVarGetID (MDVarWTemp_GrdWater, "degC", MFOutput, MFState, MFBoundary))  == CMfailed) ||
		(MFModelAddFunction(_MDWTemp_GrdWater) == CMfailed)) return (CMfailed);

	MFDefLeaving ("Groundwater temperature");
	return (_MDOutWTemp_GrdWaterID);
}
