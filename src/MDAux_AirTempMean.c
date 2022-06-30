/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDAux_MeanAirTemperature.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <MF.h>
#include <MD.h>

static int _MDInAux_AirTemperatureID      = MFUnset;
static int _MDInAux_StepCounterID         = MFUnset;

static int _MDOutAux_AirTemperatureMeanID = MFUnset;

static void _MDAux_AirTemperatureMean (int itemID) {
	int   nSteps      = MFVarGetInt   (_MDInAux_StepCounterID,         itemID,   0);
	float airTemp     = MFVarGetFloat (_MDInAux_AirTemperatureID,      itemID, 0.0);
	float airTempMean = MFVarGetFloat (_MDOutAux_AirTemperatureMeanID, itemID, 0.0);

	airTempMean = (float) (((double) airTempMean * (double) nSteps + airTemp) / ((double) (nSteps + 1)));
	MFVarSetFloat (_MDOutAux_AirTemperatureMeanID, itemID, airTempMean);
}

int MDAux_AirTemperatureMeanDef () {
	int  optID = MFinput;
	const char *optStr;

	if (_MDOutAux_AirTemperatureMeanID != MFUnset) return (_MDOutAux_AirTemperatureMeanID);

	MFDefEntering ("Mean Air Temperature");
	if ((optStr = MFOptionGet (MDVarAux_AirTemperatureMean)) != (char *) NULL) optID = CMoptLookup (MFsourceOptions, optStr, true);
	switch (optID) {
		default:
		case MFhelp:  MFOptionMessage (MDVarAux_AirTemperatureMean, optStr, MFsourceOptions); return (CMfailed);
		case MFinput: _MDOutAux_AirTemperatureMeanID = MFVarGetID (MDVarAux_AirTemperatureMean, "degC", MFInput, MFState, MFInitial); break;
		case MFcalculate:
			if (((_MDInAux_StepCounterID         = MDAux_StepCounterDef())        == CMfailed) ||
			    ((_MDInAux_AirTemperatureID      = MDCommon_AirTemperatureDef ()) == CMfailed) ||
                ((_MDOutAux_AirTemperatureMeanID = MFVarGetID (MDVarAux_AirTemperatureMean, "degC", MFOutput, MFState, MFInitial)) == CMfailed) ||
                (MFModelAddFunction(_MDAux_AirTemperatureMean) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Mean Air Temperature");
	return (_MDOutAux_AirTemperatureMeanID);
}